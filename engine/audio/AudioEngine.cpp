#include "AudioEngine.hpp"
#include "tts/DeepgramTTS.hpp"
#include "voice_agent/DeepgramVoiceAgent.hpp"

namespace engine {
namespace audio {

AudioEngine::AudioEngine() = default;

void AudioEngine::setConfig(const AudioConfig& config) {
    config_ = config;
    config_.syncToDeepgram();
}

void AudioEngine::setTTS(std::unique_ptr<ITextToSpeech> tts) {
    tts_ = std::move(tts);
    if (logger_)
        if (auto* dg = dynamic_cast<DeepgramTTS*>(tts_.get()))
            dg->setLogger(logger_);
}

void AudioEngine::setVoiceAgent(std::unique_ptr<IVoiceAgent> agent) {
    voiceAgent_ = std::move(agent);
    if (logger_)
        if (auto* dg = dynamic_cast<DeepgramVoiceAgent*>(voiceAgent_.get()))
            dg->setLogger(logger_);
}

void AudioEngine::setLogger(IAudioLogger* logger) {
    logger_ = logger;
    if (auto* tts = dynamic_cast<DeepgramTTS*>(tts_.get()))
        tts->setLogger(logger);
    if (auto* va = dynamic_cast<DeepgramVoiceAgent*>(voiceAgent_.get()))
        va->setLogger(logger);
}

}  // namespace audio
}  // namespace engine
