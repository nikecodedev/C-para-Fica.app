/**
 * JNI bridge for Android audio. Implements C++ transport interfaces
 * by delegating to Kotlin AudioBridge (OkHttp, AudioTrack, AudioRecord).
 */
#include "../../../engine/audio/AudioEngine.hpp"
#include "../../../engine/audio/tts/DeepgramTTS.hpp"
#include "../../../engine/audio/voice_agent/DeepgramVoiceAgent.hpp"
#include "../../../engine/audio/transport/IHttpTransport.hpp"
#include "../../../engine/audio/transport/IWebSocketTransport.hpp"
#include "../../../engine/audio/transport/IAudioSink.hpp"
#include "../../../engine/audio/transport/IAudioSource.hpp"
#include "../../../engine/audio/transport/ITimer.hpp"
#include <jni.h>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include <atomic>

static JavaVM* g_vm = nullptr;

static JNIEnv* getEnv() {
    JNIEnv* env = nullptr;
    if (g_vm && g_vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) == JNI_OK)
        return env;
    return nullptr;
}

// MARK: - Android IHttpTransport

class AndroidHttpTransport : public engine::audio::IHttpTransport {
public:
    AndroidHttpTransport(JNIEnv* env, jobject bridge) {
        env->GetJavaVM(&g_vm);
        bridge_ = env->NewGlobalRef(bridge);
        jclass c = env->GetObjectClass(bridge);
        httpPost_ = env->GetMethodID(c, "httpPost", "(JLjava/lang/String;[Ljava/lang/String;[Ljava/lang/String;[B)V");
        env->DeleteLocalRef(c);
    }
    ~AndroidHttpTransport() override {
        if (JNIEnv* env = getEnv()) env->DeleteGlobalRef(bridge_);
    }
    void post(const std::string& url, const std::vector<Header>& headers,
              const std::vector<std::uint8_t>& body, OnComplete onComplete) override {
        std::lock_guard<std::mutex> lock(mutex_);
        callback_ = onComplete;
        JNIEnv* env = getEnv();
        if (!env || !httpPost_) return;
        jstring jUrl = env->NewStringUTF(url.c_str());
        jclass stringClass = env->FindClass("java/lang/String");
        jobjectArray jKeys = env->NewObjectArray(static_cast<jsize>(headers.size()), stringClass, nullptr);
        jobjectArray jVals = env->NewObjectArray(static_cast<jsize>(headers.size()), stringClass, nullptr);
        for (size_t i = 0; i < headers.size(); ++i) {
            env->SetObjectArrayElement(jKeys, static_cast<jint>(i), env->NewStringUTF(headers[i].first.c_str()));
            env->SetObjectArrayElement(jVals, static_cast<jint>(i), env->NewStringUTF(headers[i].second.c_str()));
        }
        jbyteArray jBody = env->NewByteArray(static_cast<jsize>(body.size()));
        if (!body.empty())
            env->SetByteArrayRegion(jBody, 0, static_cast<jsize>(body.size()),
                reinterpret_cast<const jbyte*>(body.data()));
        env->CallVoidMethod(bridge_, httpPost_, reinterpret_cast<jlong>(this), jUrl, jKeys, jVals, jBody);
        env->DeleteLocalRef(jUrl);
        env->DeleteLocalRef(jKeys);
        env->DeleteLocalRef(jVals);
        env->DeleteLocalRef(jBody);
        env->DeleteLocalRef(stringClass);
    }
    void onComplete(bool success, int statusCode, const std::vector<std::uint8_t>& body) {
        OnComplete cb;
        { std::lock_guard<std::mutex> lock(mutex_); cb = std::move(callback_); }
        if (cb) cb(success, statusCode, body);
    }
private:
    jobject bridge_;
    jmethodID httpPost_;
    OnComplete callback_;
    std::mutex mutex_;
};

// MARK: - Android IWebSocketTransport

class AndroidWebSocketTransport : public engine::audio::IWebSocketTransport {
public:
    AndroidWebSocketTransport(JNIEnv* env, jobject bridge) {
        bridge_ = env->NewGlobalRef(bridge);
        jclass c = env->GetObjectClass(bridge);
        wsConnect_ = env->GetMethodID(c, "webSocketConnect", "(JLjava/lang/String;[Ljava/lang/String;[Ljava/lang/String;)V");
        wsSendBinary_ = env->GetMethodID(c, "webSocketSendBinary", "(J[B)V");
        wsSendText_ = env->GetMethodID(c, "webSocketSendText", "(JLjava/lang/String;)V");
        wsClose_ = env->GetMethodID(c, "webSocketClose", "(J)V");
        env->DeleteLocalRef(c);
    }
    void connect(const std::string& url, const std::vector<std::pair<std::string, std::string>>& headers) override {
        JNIEnv* env = getEnv();
        if (!env) return;
        jstring jUrl = env->NewStringUTF(url.c_str());
        jclass stringClass = env->FindClass("java/lang/String");
        jobjectArray jKeys = env->NewObjectArray(static_cast<jsize>(headers.size()), stringClass, nullptr);
        jobjectArray jVals = env->NewObjectArray(static_cast<jsize>(headers.size()), stringClass, nullptr);
        for (size_t i = 0; i < headers.size(); ++i) {
            env->SetObjectArrayElement(jKeys, static_cast<jint>(i), env->NewStringUTF(headers[i].first.c_str()));
            env->SetObjectArrayElement(jVals, static_cast<jint>(i), env->NewStringUTF(headers[i].second.c_str()));
        }
        env->CallVoidMethod(bridge_, wsConnect_, reinterpret_cast<jlong>(this), jUrl, jKeys, jVals);
        env->DeleteLocalRef(jUrl);
        env->DeleteLocalRef(jKeys);
        env->DeleteLocalRef(jVals);
    }
    void sendBinary(const std::vector<std::uint8_t>& data) override {
        JNIEnv* env = getEnv();
        if (!env || !connected_) return;
        jbyteArray jData = env->NewByteArray(static_cast<jsize>(data.size()));
        if (!data.empty())
            env->SetByteArrayRegion(jData, 0, static_cast<jsize>(data.size()),
                reinterpret_cast<const jbyte*>(data.data()));
        env->CallVoidMethod(bridge_, wsSendBinary_, reinterpret_cast<jlong>(this), jData);
        env->DeleteLocalRef(jData);
    }
    void sendText(const std::string& text) override {
        JNIEnv* env = getEnv();
        if (!env || !connected_) return;
        jstring jText = env->NewStringUTF(text.c_str());
        env->CallVoidMethod(bridge_, wsSendText_, reinterpret_cast<jlong>(this), jText);
        env->DeleteLocalRef(jText);
    }
    void close() override {
        if (JNIEnv* env = getEnv())
            env->CallVoidMethod(bridge_, wsClose_, reinterpret_cast<jlong>(this));
        connected_ = false;
    }
    bool isConnected() const override { return connected_; }
    void setOnText(OnText cb) override { onText_ = cb; }
    void setOnBinary(OnBinary cb) override { onBinary_ = cb; }
    void setOnOpen(OnOpen cb) override { onOpen_ = cb; }
    void setOnClose(OnClose cb) override { onClose_ = cb; }
    void onOpen() { connected_ = true; if (onOpen_) onOpen_(); }
    void onClose(const std::string& r) { connected_ = false; if (onClose_) onClose_(r); }
    void onText(const std::string& t) { if (onText_) onText_(t); }
private:
    jobject bridge_;
    jmethodID wsConnect_, wsSendBinary_, wsSendText_, wsClose_;
    std::atomic<bool> connected_{false};
    OnText onText_; OnBinary onBinary_; OnOpen onOpen_; OnClose onClose_;
};

// MARK: - Android IAudioSink

class AndroidAudioSink : public engine::audio::IAudioSink {
public:
    AndroidAudioSink(JNIEnv* env, jobject bridge) : bridge_(env->NewGlobalRef(bridge)) {
        jclass c = env->GetObjectClass(bridge);
        playPcm_ = env->GetMethodID(c, "playPcm", "(I[B)V");
        env->DeleteLocalRef(c);
    }
    void play(const std::vector<std::uint8_t>& pcmData) override {
        JNIEnv* env = getEnv();
        if (!env) return;
        jbyteArray jData = env->NewByteArray(static_cast<jsize>(pcmData.size()));
        if (!pcmData.empty())
            env->SetByteArrayRegion(jData, 0, static_cast<jsize>(pcmData.size()),
                reinterpret_cast<const jbyte*>(pcmData.data()));
        env->CallVoidMethod(bridge_, playPcm_, sampleRate_, jData);
        env->DeleteLocalRef(jData);
    }
    void stop() override {}
    int getSampleRate() const override { return sampleRate_; }
    void setSampleRate(int r) { sampleRate_ = r; }
private:
    jobject bridge_;
    jmethodID playPcm_;
    int sampleRate_{24000};
};

// MARK: - Android IAudioSource

class AndroidAudioSource : public engine::audio::IAudioSource {
public:
    AndroidAudioSource(JNIEnv* env, jobject bridge) : bridge_(env->NewGlobalRef(bridge)) {
        jclass c = env->GetObjectClass(bridge);
        startCapture_ = env->GetMethodID(c, "startCapture", "(JII)V");
        stopCapture_ = env->GetMethodID(c, "stopCapture", "(J)V");
        env->DeleteLocalRef(c);
    }
    void startCapture(OnAudioData onData) override {
        onData_ = onData;
        JNIEnv* env = getEnv();
        if (!env) return;
        env->CallVoidMethod(bridge_, startCapture_, reinterpret_cast<jlong>(this), sampleRate_, 2048);
    }
    void stopCapture() override {
        if (JNIEnv* env = getEnv())
            env->CallVoidMethod(bridge_, stopCapture_, reinterpret_cast<jlong>(this));
    }
    int getSampleRate() const override { return sampleRate_; }
    void onData(const std::vector<std::uint8_t>& v) { if (onData_) onData_(v); }
private:
    jobject bridge_;
    jmethodID startCapture_, stopCapture_;
    int sampleRate_{16000};
    OnAudioData onData_;
};

// MARK: - Android ITimer

class AndroidTimer : public engine::audio::ITimer {
public:
    AndroidTimer(JNIEnv* env, jobject bridge) : bridge_(env->NewGlobalRef(bridge)) {
        jclass c = env->GetObjectClass(bridge);
        startTimer_ = env->GetMethodID(c, "startTimer", "(JJ)V");
        stopTimer_ = env->GetMethodID(c, "stopTimer", "(J)V");
        env->DeleteLocalRef(c);
    }
    void start(std::uint32_t intervalMs, std::function<void()> callback) override {
        callback_ = callback;
        if (JNIEnv* env = getEnv())
            env->CallVoidMethod(bridge_, startTimer_, reinterpret_cast<jlong>(this), static_cast<jlong>(intervalMs));
    }
    void stop() override {
        if (JNIEnv* env = getEnv())
            env->CallVoidMethod(bridge_, stopTimer_, reinterpret_cast<jlong>(this));
        callback_ = nullptr;
    }
    void onTick() { if (callback_) callback_(); }
private:
    jobject bridge_;
    jmethodID startTimer_, stopTimer_;
    std::function<void()> callback_;
};

// MARK: - Audio engine handle

struct AndroidAudioHandle {
    jobject bridge;
    jobject commandListener{nullptr};
    jmethodID onCommandMethod{nullptr};
    std::shared_ptr<AndroidHttpTransport> http;
    std::shared_ptr<AndroidWebSocketTransport> ws;
    std::shared_ptr<AndroidAudioSink> sink;
    std::shared_ptr<AndroidAudioSource> source;
    std::shared_ptr<AndroidTimer> timer;
    std::unique_ptr<engine::audio::AudioEngine> engine;
    engine::audio::AudioConfig config;
};

static AndroidAudioHandle* getAudioHandle(jlong ptr) {
    return reinterpret_cast<AndroidAudioHandle*>(static_cast<intptr_t>(ptr));
}

// MARK: - JNI exports (called from Kotlin)

extern "C" {

JNIEXPORT jlong JNICALL
Java_com_ficamotor_platform_audio_AudioBridge_nativeInit(JNIEnv* env, jobject thiz) {
    auto* h = new AndroidAudioHandle();
    env->GetJavaVM(&g_vm);
    h->bridge = env->NewGlobalRef(thiz);
    h->http = std::make_shared<AndroidHttpTransport>(env, thiz);
    h->ws = std::make_shared<AndroidWebSocketTransport>(env, thiz);
    h->sink = std::make_shared<AndroidAudioSink>(env, thiz);
    h->source = std::make_shared<AndroidAudioSource>(env, thiz);
    h->timer = std::make_shared<AndroidTimer>(env, thiz);
    h->config.sampleRate = 16000;
    h->config.ttsVoice = "aura-asteria-en";
    h->config.language = "en";
    h->engine = std::make_unique<engine::audio::AudioEngine>();
    h->engine->setConfig(h->config);
    h->engine->setTTS(std::make_unique<engine::audio::DeepgramTTS>(h->http, h->sink, h->config));
    h->engine->setVoiceAgent(std::make_unique<engine::audio::DeepgramVoiceAgent>(h->ws, h->source, h->timer, h->config));
    return static_cast<jlong>(reinterpret_cast<intptr_t>(h));
}

JNIEXPORT void JNICALL
Java_com_ficamotor_platform_audio_AudioBridge_nativeDispose(JNIEnv* env, jobject, jlong ptr) {
    auto* h = getAudioHandle(ptr);
    if (h) {
        env->DeleteGlobalRef(h->bridge);
        delete h;
    }
}

JNIEXPORT void JNICALL
Java_com_ficamotor_platform_audio_AudioBridge_nativeSetApiKey(JNIEnv*, jobject, jlong ptr, jstring key) {
    auto* h = getAudioHandle(ptr);
    if (!h) return;
    JNIEnv* env = getEnv();
    if (!env || !key) return;
    const char* c = env->GetStringUTFChars(key, nullptr);
    h->config.apiKey = c;
    env->ReleaseStringUTFChars(key, c);
    h->engine->setConfig(h->config);
}

JNIEXPORT void JNICALL
Java_com_ficamotor_platform_audio_AudioBridge_nativeSpeak(JNIEnv* env, jobject, jlong ptr, jstring text) {
    auto* h = getAudioHandle(ptr);
    if (!h || !h->engine->getTTS()) return;
    const char* c = env->GetStringUTFChars(text, nullptr);
    h->engine->getTTS()->speak(c ?: "");
    env->ReleaseStringUTFChars(text, c);
}

JNIEXPORT void JNICALL
Java_com_ficamotor_platform_audio_AudioBridge_nativeStartListening(JNIEnv*, jobject, jlong ptr) {
    auto* h = getAudioHandle(ptr);
    if (h && h->engine->getVoiceAgent()) h->engine->getVoiceAgent()->startListening();
}

JNIEXPORT void JNICALL
Java_com_ficamotor_platform_audio_AudioBridge_nativeStopListening(JNIEnv*, jobject, jlong ptr) {
    auto* h = getAudioHandle(ptr);
    if (h && h->engine->getVoiceAgent()) h->engine->getVoiceAgent()->stopListening();
}

JNIEXPORT void JNICALL
Java_com_ficamotor_platform_audio_AudioBridge_nativeSetOnCommand(JNIEnv* env, jobject, jlong ptr, jobject listener) {
    auto* h = getAudioHandle(ptr);
    if (!h) return;
    if (h->commandListener) env->DeleteGlobalRef(h->commandListener);
    h->commandListener = listener ? env->NewGlobalRef(listener) : nullptr;
    jclass c = env->GetObjectClass(listener);
    h->onCommandMethod = env->GetMethodID(c, "onCommand", "(Ljava/lang/String;)V");
    env->DeleteLocalRef(c);
    auto* va = dynamic_cast<engine::audio::DeepgramVoiceAgent*>(h->engine->getVoiceAgent());
    if (!va) return;
    va->setOnCommand([h](const std::string& cmd) {
        JNIEnv* e = getEnv();
        if (!e || !h->commandListener || !h->onCommandMethod) return;
        jstring jCmd = e->NewStringUTF(cmd.c_str());
        e->CallVoidMethod(h->commandListener, h->onCommandMethod, jCmd);
        e->DeleteLocalRef(jCmd);
    });
}

JNIEXPORT void JNICALL
Java_com_ficamotor_platform_audio_AudioBridge_nativeOnHttpComplete(JNIEnv* env, jclass, jlong ptr, jboolean success, jint statusCode, jbyteArray body) {
    auto* t = reinterpret_cast<AndroidHttpTransport*>(static_cast<intptr_t>(ptr));
    std::vector<std::uint8_t> vec;
    if (body) {
        jsize len = env->GetArrayLength(body);
        vec.resize(static_cast<size_t>(len));
        if (len > 0)
            env->GetByteArrayRegion(body, 0, len, reinterpret_cast<jbyte*>(vec.data()));
    }
    if (t) t->onComplete(success == JNI_TRUE, statusCode, vec);
}

JNIEXPORT void JNICALL
Java_com_ficamotor_platform_audio_AudioBridge_nativeOnWebSocketOpen(JNIEnv*, jclass, jlong ptr) {
    auto* t = reinterpret_cast<AndroidWebSocketTransport*>(static_cast<intptr_t>(ptr));
    if (t) t->onOpen();
}

JNIEXPORT void JNICALL
Java_com_ficamotor_platform_audio_AudioBridge_nativeOnWebSocketClose(JNIEnv* env, jclass, jlong ptr, jstring reason) {
    auto* t = reinterpret_cast<AndroidWebSocketTransport*>(static_cast<intptr_t>(ptr));
    std::string r;
    if (reason) {
        const char* c = env->GetStringUTFChars(reason, nullptr);
        if (c) { r = c; env->ReleaseStringUTFChars(reason, c); }
    }
    if (t) t->onClose(r);
}

JNIEXPORT void JNICALL
Java_com_ficamotor_platform_audio_AudioBridge_nativeOnWebSocketText(JNIEnv* env, jclass, jlong ptr, jstring text) {
    auto* t = reinterpret_cast<AndroidWebSocketTransport*>(static_cast<intptr_t>(ptr));
    std::string s;
    if (text) {
        const char* c = env->GetStringUTFChars(text, nullptr);
        if (c) s = c;
        env->ReleaseStringUTFChars(text, c);
    }
    if (t) t->onText(s);
}

JNIEXPORT void JNICALL
Java_com_ficamotor_platform_audio_AudioBridge_nativeOnAudioData(JNIEnv* env, jclass, jlong ptr, jbyteArray data) {
    auto* t = reinterpret_cast<AndroidAudioSource*>(static_cast<intptr_t>(ptr));
    std::vector<std::uint8_t> vec;
    if (data) {
        jsize len = env->GetArrayLength(data);
        vec.resize(static_cast<size_t>(len));
        if (len > 0)
            env->GetByteArrayRegion(data, 0, len, reinterpret_cast<jbyte*>(vec.data()));
    }
    if (t) t->onData(vec);
}

JNIEXPORT void JNICALL
Java_com_ficamotor_platform_audio_AudioBridge_nativeOnTimerTick(JNIEnv*, jclass, jlong ptr) {
    auto* t = reinterpret_cast<AndroidTimer*>(static_cast<intptr_t>(ptr));
    if (t) t->onTick();
}

}
