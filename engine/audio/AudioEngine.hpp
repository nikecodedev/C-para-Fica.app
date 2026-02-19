#pragma once

#include "tts/ITextToSpeech.hpp"
#include "voice_agent/IVoiceAgent.hpp"
#include "config/AudioConfig.hpp"
#include "logging/AudioLogger.hpp"
#include <memory>

namespace engine {
namespace audio {

/**
 * Audio engine facade. Platform-agnostic.
 * Platform layer provides TTS and VoiceAgent implementations.
 */
class AudioEngine {
public:
    AudioEngine();

    void setConfig(const AudioConfig& config);
    const AudioConfig& getConfig() const { return config_; }

    void setTTS(std::unique_ptr<ITextToSpeech> tts);
    ITextToSpeech* getTTS() { return tts_.get(); }
    const ITextToSpeech* getTTS() const { return tts_.get(); }

    void setVoiceAgent(std::unique_ptr<IVoiceAgent> agent);
    IVoiceAgent* getVoiceAgent() { return voiceAgent_.get(); }
    const IVoiceAgent* getVoiceAgent() const { return voiceAgent_.get(); }

    void setLogger(IAudioLogger* logger);

private:
    AudioConfig config_;
    std::unique_ptr<ITextToSpeech> tts_;
    std::unique_ptr<IVoiceAgent> voiceAgent_;
    IAudioLogger* logger_{nullptr};
};

}  // namespace audio
}  // namespace engine
