#pragma once

/** Stub transport implementations for unit tests. No real I/O. */

#include "../transport/IHttpTransport.hpp"
#include "../transport/IWebSocketTransport.hpp"
#include "../transport/IAudioSink.hpp"
#include "../transport/IAudioSource.hpp"
#include "../transport/ITimer.hpp"
#include <atomic>

namespace engine {
namespace audio {

class StubHttpTransport : public IHttpTransport {
public:
    void post(const std::string&,
              const std::vector<Header>&,
              const std::vector<std::uint8_t>& body,
              OnComplete onComplete) override {
        postCount_++;
        lastBody_.assign(body.begin(), body.end());
        if (onComplete) onComplete(success_, statusCode_, responseBody_);
    }
    std::atomic<int> postCount_{0};
    std::string lastBody_;
    bool success_{true};
    int statusCode_{200};
    std::vector<std::uint8_t> responseBody_;
};

class StubAudioSink : public IAudioSink {
public:
    void play(const std::vector<std::uint8_t>& data) override {
        playCount_++;
        lastData_ = data;
    }
    void stop() override {}
    int getSampleRate() const override { return 24000; }
    std::atomic<int> playCount_{0};
    std::vector<std::uint8_t> lastData_;
};

class StubWebSocketTransport : public IWebSocketTransport {
public:
    void connect(const std::string&, const std::vector<std::pair<std::string, std::string>>&) override {
        if (onOpen_) onOpen_();
    }
    void sendBinary(const std::vector<std::uint8_t>& data) override {
        binarySendCount_++;
        totalBytesSent_ += data.size();
    }
    void sendText(const std::string&) override {}
    void close() override {
        if (onClose_) onClose_("closed");
    }
    bool isConnected() const override { return connected_; }
    void setOnText(OnText cb) override { onText_ = cb; }
    void setOnBinary(OnBinary cb) override { onBinary_ = cb; }
    void setOnOpen(OnOpen cb) override { onOpen_ = cb; }
    void setOnClose(OnClose cb) override { onClose_ = cb; }
    bool connected_{true};
    std::atomic<int> binarySendCount_{0};
    std::atomic<size_t> totalBytesSent_{0};
    OnText onText_;
    OnBinary onBinary_;
    OnOpen onOpen_;
    OnClose onClose_;
};

class StubAudioSource : public IAudioSource {
public:
    void startCapture(OnAudioData onData) override { onData_ = onData; }
    void stopCapture() override {}
    int getSampleRate() const override { return 16000; }
    OnAudioData onData_;
};

class StubTimer : public ITimer {
public:
    void start(std::uint32_t, std::function<void()> cb) override { callback_ = cb; }
    void stop() override {}
    void fire() { if (callback_) callback_(); }
    std::function<void()> callback_;
};

}  // namespace audio
}  // namespace engine
