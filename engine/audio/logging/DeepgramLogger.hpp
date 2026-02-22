#pragma once

#include "AudioLogger.hpp"
#include <chrono>
#include <string>

namespace engine {
namespace audio {

/**
 * Structured logging for Deepgram integration.
 * Wraps IAudioLogger; logs TTS requests, voice commands, API responses, latency.
 */
class DeepgramLogger {
public:
    DeepgramLogger() : delegate_(nullptr) {}
    explicit DeepgramLogger(IAudioLogger* delegate) : delegate_(delegate) {}

    void setDelegate(IAudioLogger* d) { delegate_ = d; }
    IAudioLogger* getDelegate() const { return delegate_; }

    /** Log TTS request (text truncated for privacy). */
    void logTtsRequest(const std::string& text) {
        if (!delegate_) return;
        std::string preview = text.substr(0, 64);
        if (text.size() > 64) preview += "...";
        delegate_->log(IAudioLogger::Level::Info, "[TTS] request: " + preview + " (len=" + std::to_string(text.size()) + ")");
    }

    /** Log voice command received. */
    void logVoiceCommand(const std::string& command) {
        if (!delegate_) return;
        delegate_->log(IAudioLogger::Level::Info, "[Voice] command: " + command);
    }

    /** Log Deepgram API response (TTS or STT). */
    void logApiResponse(bool success, int statusCode, size_t bodySizeBytes, int latencyMs = -1) {
        if (!delegate_) return;
        std::string msg = "[API] " + std::string(success ? "OK" : "FAIL") +
            " status=" + std::to_string(statusCode) +
            " body=" + std::to_string(bodySizeBytes) + "B";
        if (latencyMs >= 0) msg += " latency=" + std::to_string(latencyMs) + "ms";
        delegate_->log(success ? IAudioLogger::Level::Info : IAudioLogger::Level::Error, msg);
    }

    /** Log latency metric. */
    void logLatency(const std::string& operation, int ms) {
        if (!delegate_) return;
        delegate_->log(IAudioLogger::Level::Debug, "[Latency] " + operation + ": " + std::to_string(ms) + "ms");
    }

    /** Log WebSocket/streaming event. */
    void logStreamEvent(const std::string& event, const std::string& detail = {}) {
        if (!delegate_) return;
        std::string msg = "[Stream] " + event;
        if (!detail.empty()) msg += ": " + detail;
        delegate_->log(IAudioLogger::Level::Debug, msg);
    }

    /** Return current timestamp for latency calculation. */
    static int nowMs() {
        using namespace std::chrono;
        return static_cast<int>(duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count());
    }

private:
    IAudioLogger* delegate_{nullptr};
};

}  // namespace audio
}  // namespace engine
