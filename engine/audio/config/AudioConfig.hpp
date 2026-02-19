#pragma once

#include "DeepgramConfig.hpp"
#include <string>

namespace engine {
namespace audio {

/**
 * Audio engine configuration. Platform-agnostic.
 * API key must be set via setApiKey() from secure storage (platform layer).
 * Never hardcode API keys in source.
 */
struct AudioConfig {
    std::string apiKey;
    std::string language{"en"};
    std::string ttsModel{"aura-asteria-en"};
    std::string ttsVoice{"aura-asteria-en"};
    int sampleRate{16000};
    bool useSecureStorage{true};

    /** Deepgram-specific settings (mode, defaults). */
    DeepgramConfig deepgram;

    /** Sync apiKey/language/voice into deepgram for components that use it. */
    void syncToDeepgram() {
        deepgram.apiKey = apiKey;
        deepgram.language = language;
        deepgram.ttsVoice = ttsVoice;
        deepgram.sttSampleRate = (sampleRate > 0) ? sampleRate : 16000;
    }
};

}  // namespace audio
}  // namespace engine
