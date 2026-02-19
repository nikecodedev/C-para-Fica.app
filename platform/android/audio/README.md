# Android Audio Bridge

Platform layer for Deepgram TTS and Voice Agent. Uses OkHttp, AudioTrack, AudioRecord.

## Components

| Component | Implementation |
|-----------|-----------------|
| **IHttpTransport** | OkHttp POST |
| **IWebSocketTransport** | OkHttp WebSocket |
| **IAudioSink** | AudioTrack (linear16 PCM) |
| **IAudioSource** | AudioRecord VOICE_RECOGNITION |
| **ITimer** | Handler postDelayed |

## Usage

```kotlin
val audio = AudioBridge(context)
audio.setApiKey("your-deepgram-api-key")  // From Keystore

audio.setOnCommand(object : OnCommandListener {
    override fun onCommand(cmd: String) {
        Log.d("Audio", "Voice command: $cmd")
    }
})

audio.speak("Hello world")
audio.startListening()
// ... later
audio.stopListening()
audio.dispose()
```

## Dependencies

Add to build.gradle:
```groovy
implementation 'com.squareup.okhttp3:okhttp:4.x.x'
```

## Permissions

- `RECORD_AUDIO` for mic capture
- `INTERNET` for Deepgram API

## Native Build

Include `platform/android/audio/AndroidAudioJNI.cpp` in your native library (CMake/Android.mk) and link with `engine_audio`.
