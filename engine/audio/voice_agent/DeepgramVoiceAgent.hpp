#pragma once

#include "IVoiceAgent.hpp"
#include "../config/AudioConfig.hpp"
#include "../transport/IWebSocketTransport.hpp"
#include "../transport/IAudioSource.hpp"
#include "../transport/ITimer.hpp"
#include "../logging/AudioLogger.hpp"
#include <memory>
#include <mutex>

namespace engine {
namespace audio {

/**
 * Deepgram streaming Voice Agent. Uses WebSocket to wss://api.deepgram.com/v1/listen.
 * Streams live mic audio, receives transcripts, invokes onCommand for final utterances.
 * Platform provides IWebSocketTransport, IAudioSource, ITimer.
 * KeepAlive sent every 4s during silence for low-latency connection.
 */
class DeepgramVoiceAgent : public IVoiceAgent {
public:
    DeepgramVoiceAgent(std::shared_ptr<IWebSocketTransport> ws,
                       std::shared_ptr<IAudioSource> audioSource,
                       std::shared_ptr<ITimer> timer,
                       const AudioConfig& config);

    ~DeepgramVoiceAgent() override;

    void startListening() override;
    void stopListening() override;
    void setOnCommand(OnCommand callback) override;

    void setConfig(const AudioConfig& config);
    void setLogger(IAudioLogger* logger) { logger_ = logger; }

private:
    void connectWebSocket();
    void onTextMessage(const std::string& text);
    void onAudioChunk(const std::vector<std::uint8_t>& chunk);
    void sendKeepAlive();
    std::string extractTranscript(const std::string& json) const;
    bool extractSpeechFinal(const std::string& json) const;

    std::shared_ptr<IWebSocketTransport> ws_;
    std::shared_ptr<IAudioSource> audioSource_;
    std::shared_ptr<ITimer> timer_;
    AudioConfig config_;
    IAudioLogger* logger_{nullptr};

    OnCommand onCommand_;
    std::mutex mutex_;
    bool listening_{false};
    static constexpr std::uint32_t KEEPALIVE_MS = 4000;
};

}  // namespace audio
}  // namespace engine
