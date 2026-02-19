#pragma once

#include "engine/audio/tts/ITextToSpeech.hpp"
#include "localization/Localizer.hpp"
#include "engine/audio/tts/TTSConstants.hpp"
#include <string>

namespace app {
namespace audio {

/**
 * Syncs TTS language/voice with the app Localizer.
 * Call when locale changes (startup, user settings, region change).
 */
class TTSLocalizationSync {
public:
    /**
     * Apply localizer's current locale to TTS.
     * en → language "en", voice aura-asteria-en
     * pt → language "pt", voice aura-luna-pt
     */
    static void sync(const app::localization::Localizer& localizer,
                     engine::audio::ITextToSpeech* tts);

    /**
     * Sync from system locale string (e.g. "pt-BR", "en-US").
     * Resolves to "en" or "pt" and configures TTS.
     */
    static void syncFromSystem(const std::string& systemLocale,
                               engine::audio::ITextToSpeech* tts);

    /** Get TTS voice ID for locale ("en" or "pt"). */
    static const char* voiceForLocale(const std::string& locale);

    /** Get TTS language code for locale. */
    static const char* languageForLocale(const std::string& locale);
};

}  // namespace audio
}  // namespace app
