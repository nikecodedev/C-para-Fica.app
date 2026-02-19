#pragma once

#include "ITextToSpeech.hpp"
#include "../config/AudioConfig.hpp"
#include "../transport/IHttpTransport.hpp"
#include "../transport/IAudioSink.hpp"
#include "../logging/AudioLogger.hpp"
#include <memory>
#include <string>

namespace engine {
namespace audio {

/**
 * Deepgram TTS implementation. Uses REST API: POST /v1/speak.
 * Platform provides IHttpTransport and IAudioSink.
 * Low-latency: requests linear16 PCM for immediate playback.
 */
class DeepgramTTS : public ITextToSpeech {
public:
    DeepgramTTS(std::shared_ptr<IHttpTransport> http,
                std::shared_ptr<IAudioSink> audioSink,
                const AudioConfig& config);

    void speak(const std::string& text) override;
    void setLanguage(const std::string& lang) override;
    void setVoice(const std::string& voice) override;

    void setConfig(const AudioConfig& config) { config_ = config; }
    void setLogger(IAudioLogger* logger) { logger_ = logger; }

private:
    std::string buildRequestUrl() const;
    std::vector<std::uint8_t> buildRequestBody(const std::string& text) const;

    std::shared_ptr<IHttpTransport> http_;
    std::shared_ptr<IAudioSink> audioSink_;
    AudioConfig config_;
    IAudioLogger* logger_{nullptr};
};

}  // namespace audio
}  // namespace engine
