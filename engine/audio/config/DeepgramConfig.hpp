#pragma once

#include <string>
#include <cstring>

namespace engine {
namespace audio {

/** Real-time (low latency) vs production (quality) mode for Deepgram. */
enum class DeepgramMode {
    /** Low latency: interim results, shorter utterance end. */
    RealTime,
    /** Higher quality: final results only, longer utterance end. */
    Production
};

/**
 * Deepgram-specific configuration. Platform must supply API key from secure storage
 * (Keychain/Keystore). Never hardcode or log the key.
 */
struct DeepgramConfig {
    /** API key. Set from platform secure storage. Cleared on clear(). */
    std::string apiKey;

    /** Default language (e.g. "en", "pt"). */
    std::string language{"en"};

    /** Default TTS voice (e.g. "aura-asteria-en", "aura-luna-pt"). */
    std::string ttsVoice{"aura-asteria-en"};

    /** Sample rate for TTS output (8000 or 24000). */
    int ttsSampleRate{24000};

    /** Sample rate for voice input (e.g. 16000). */
    int sttSampleRate{16000};

    /** Real-time vs production mode. */
    DeepgramMode mode{DeepgramMode::RealTime};

    /** Clear API key from memory. Call when done. */
    void clearApiKey() {
        if (!apiKey.empty()) {
            std::memset(apiKey.data(), 0, apiKey.size());
            apiKey.clear();
        }
    }

    /** Real-time: interim_results=true, utterance_end_ms=800 (min latency). Production: 2000. */
    bool useInterimResults() const { return mode == DeepgramMode::RealTime; }
    int utteranceEndMs() const { return mode == DeepgramMode::RealTime ? 800 : 2000; }
};

}  // namespace audio
}  // namespace engine
