package com.ficamotor.platform.audio

import android.content.Context
import android.media.AudioFormat
import android.media.AudioRecord
import android.media.AudioTrack
import android.media.MediaRecorder
import android.os.Handler
import android.os.Looper
import okhttp3.*
import okhttp3.MediaType.Companion.toMediaType
import okhttp3.RequestBody.Companion.toRequestBody
import java.io.IOException
import java.util.concurrent.ConcurrentHashMap
import java.util.concurrent.atomic.AtomicBoolean

/** Callback for voice commands. */
interface OnCommandListener {
    fun onCommand(cmd: String)
}

/**
 * Android audio bridge. Uses OkHttp (HTTP/WebSocket), AudioTrack (TTS playback),
 * AudioRecord (mic capture). Forwards to C++ via JNI callbacks.
 */
class AudioBridge(private val context: Context) {

    private var handle: Long = 0
    private var commandListener: OnCommandListener? = null

    init {
        handle = nativeInit(this)
    }

    fun setApiKey(key: String) { nativeSetApiKey(handle, key) }
    fun speak(text: String) { nativeSpeak(handle, text) }
    fun startListening() { nativeStartListening(handle) }
    fun stopListening() { nativeStopListening(handle) }
    fun setOnCommand(listener: OnCommandListener) {
        commandListener = listener
        nativeSetOnCommand(handle, listener)
    }
    fun dispose() {
        if (handle != 0L) { nativeDispose(handle); handle = 0 }
    }

    @Suppress("unused") // Called from native
    fun onCommandReceived(cmd: String) {
        commandListener?.onCommand(cmd)
    }

    private val handler = Handler(Looper.getMainLooper())
    private val httpClient = OkHttpClient()
    private val wsConnections = ConcurrentHashMap<Long, WebSocket>()
    private val captureCancel = ConcurrentHashMap<Long, AtomicBoolean>()
    private val timerRunnables = ConcurrentHashMap<Long, Runnable>()

    companion object {
        init { System.loadLibrary("fica_native") }

        @JvmStatic
        external fun nativeOnHttpComplete(ptr: Long, success: Boolean, statusCode: Int, body: ByteArray?)

        @JvmStatic
        external fun nativeOnWebSocketOpen(ptr: Long)

        @JvmStatic
        external fun nativeOnWebSocketClose(ptr: Long, reason: String?)

        @JvmStatic
        external fun nativeOnWebSocketText(ptr: Long, text: String?)

        @JvmStatic
        external fun nativeOnAudioData(ptr: Long, data: ByteArray?)

        @JvmStatic
        external fun nativeOnTimerTick(ptr: Long)
    }

    private external fun nativeInit(bridge: AudioBridge): Long
    private external fun nativeDispose(ptr: Long)
    private external fun nativeSetApiKey(ptr: Long, key: String)
    private external fun nativeSpeak(ptr: Long, text: String)
    private external fun nativeStartListening(ptr: Long)
    private external fun nativeStopListening(ptr: Long)
    private external fun nativeSetOnCommand(ptr: Long, listener: OnCommandListener)

    /** HTTP POST - calls nativeOnHttpComplete(ptr, ...) when done. */
    fun httpPost(ptr: Long, url: String, headerKeys: Array<String>, headerValues: Array<String>, body: ByteArray) {
        val json = "application/json".toMediaType()
        val reqBody = body.toRequestBody(json)
        val builder = Request.Builder().url(url).post(reqBody)
        for (i in headerKeys.indices) {
            if (i < headerValues.size) builder.addHeader(headerKeys[i], headerValues[i])
        }
        httpClient.newCall(builder.build()).enqueue(object : Callback {
            override fun onFailure(call: Call, e: IOException) {
                nativeOnHttpComplete(ptr, false, 0, null)
            }
            override fun onResponse(call: Call, response: Response) {
                val success = response.isSuccessful
                val code = response.code
                val bytes = response.body?.bytes()
                nativeOnHttpComplete(ptr, success, code, bytes ?: ByteArray(0))
            }
        })
    }

    /** WebSocket connect. Sends binary via sendBinary, text via sendText. */
    fun webSocketConnect(ptr: Long, url: String, headerKeys: Array<String>, headerValues: Array<String>) {
        val reqBuilder = Request.Builder().url(url)
        for (i in headerKeys.indices) {
            if (i < headerValues.size) reqBuilder.addHeader(headerKeys[i], headerValues[i])
        }
        val req = reqBuilder.build()
        val listener = object : WebSocketListener() {
            override fun onOpen(webSocket: WebSocket, response: Response) {
                nativeOnWebSocketOpen(ptr)
            }
            override fun onMessage(webSocket: WebSocket, text: String) {
                nativeOnWebSocketText(ptr, text)
            }
            override fun onClosing(webSocket: WebSocket, code: Int, reason: String) {
                wsConnections.remove(ptr)
                nativeOnWebSocketClose(ptr, reason)
            }
            override fun onFailure(webSocket: WebSocket, t: Throwable, response: Response?) {
                wsConnections.remove(ptr)
                nativeOnWebSocketClose(ptr, t.message ?: "error")
            }
        }
        val ws = httpClient.newWebSocket(req, listener)
        wsConnections[ptr] = ws
    }

    fun webSocketSendBinary(ptr: Long, data: ByteArray) {
        wsConnections[ptr]?.send(okio.ByteString.of(*data))
    }


    fun webSocketSendText(ptr: Long, text: String) {
        wsConnections[ptr]?.send(text)
    }

    fun webSocketClose(ptr: Long) {
        wsConnections.remove(ptr)?.close(1000, "closed")
    }

    /** Play PCM via AudioTrack. Uses MODE_STREAM, queues data and plays. */
    fun playPcm(sampleRate: Int, data: ByteArray) {
        if (data.isEmpty()) return
        val bufSize = AudioTrack.getMinBufferSize(sampleRate, AudioFormat.CHANNEL_OUT_MONO, AudioFormat.ENCODING_PCM_16BIT)
        val track = AudioTrack.Builder()
            .setAudioFormat(android.media.AudioFormat.Builder()
                .setEncoding(AudioFormat.ENCODING_PCM_16BIT)
                .setSampleRate(sampleRate)
                .setChannelMask(AudioFormat.CHANNEL_OUT_MONO)
                .build())
            .setBufferSizeInBytes(bufSize.coerceAtLeast(data.size))
            .setTransferMode(AudioTrack.MODE_STREAM)
            .build()
        track.play()
        track.write(data, 0, data.size)
        val durationMs = (data.size * 1000L) / (sampleRate * 2)
        handler.postDelayed({
            track.stop()
            track.release()
        }, durationMs)
    }

    /** Start AudioRecord capture - calls nativeOnAudioData(ptr, chunk) with each chunk. */
    fun startCapture(ptr: Long, sampleRate: Int, bufferSize: Int) {
        captureCancel[ptr] = AtomicBoolean(false)
        val minBuf = AudioRecord.getMinBufferSize(sampleRate, AudioFormat.CHANNEL_IN_MONO, AudioFormat.ENCODING_PCM_16BIT)
        val bufSize = (minBuf * 2).coerceAtLeast(bufferSize)
        Thread {
            val rec = AudioRecord(MediaRecorder.AudioSource.VOICE_RECOGNITION, sampleRate,
                AudioFormat.CHANNEL_IN_MONO, AudioFormat.ENCODING_PCM_16BIT, bufSize)
            if (rec.state != AudioRecord.STATE_INITIALIZED) {
                rec.release()
                captureCancel.remove(ptr)
                return@Thread
            }
            rec.startRecording()
            val buf = ByteArray(bufSize)
            val cancel = captureCancel[ptr]!!
            while (!cancel.get() && rec.recordingState == AudioRecord.RECORDSTATE_RECORDING) {
                val n = rec.read(buf, 0, buf.size)
                if (n > 0) nativeOnAudioData(ptr, buf.copyOf(n))
            }
            rec.stop()
            rec.release()
            captureCancel.remove(ptr)
        }.start()
    }

    fun stopCapture(ptr: Long) {
        captureCancel[ptr]?.set(true)
    }

    /** Start recurring timer. Calls nativeOnTimerTick(ptr) every intervalMs. */
    fun startTimer(ptr: Long, intervalMs: Long) {
        stopTimer(ptr)
        val run = object : Runnable {
            override fun run() {
                if (timerRunnables[ptr] != this) return
                nativeOnTimerTick(ptr)
                handler.postDelayed(this, intervalMs)
            }
        }
        timerRunnables[ptr] = run
        handler.postDelayed(run, intervalMs)
    }

    fun stopTimer(ptr: Long) {
        timerRunnables.remove(ptr)?.let { handler.removeCallbacks(it) }
    }
}

