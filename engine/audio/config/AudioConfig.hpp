#pragma once

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
};

}  // namespace audio
}  // namespace engine
