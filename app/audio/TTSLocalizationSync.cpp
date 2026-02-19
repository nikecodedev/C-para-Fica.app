#include "TTSLocalizationSync.hpp"
#include <algorithm>
#include <cctype>

namespace app {
namespace audio {

void TTSLocalizationSync::sync(const app::localization::Localizer& localizer,
                                engine::audio::ITextToSpeech* tts) {
    if (!tts) return;
    std::string loc = localizer.getLocale();
    tts->setLanguage(languageForLocale(loc));
    tts->setVoice(voiceForLocale(loc));
}

void TTSLocalizationSync::syncFromSystem(const std::string& systemLocale,
                                          engine::audio::ITextToSpeech* tts) {
    if (!tts) return;
    std::string loc = app::localization::Localizer::resolveFromSystem(systemLocale);
    tts->setLanguage(languageForLocale(loc));
    tts->setVoice(voiceForLocale(loc));
}

const char* TTSLocalizationSync::voiceForLocale(const std::string& locale) {
    if (locale.size() >= 2) {
        char a = static_cast<char>(std::tolower(static_cast<unsigned char>(locale[0])));
        char b = static_cast<char>(std::tolower(static_cast<unsigned char>(locale[1])));
        if (a == 'p' && b == 't')
            return engine::audio::Voice::LUNA_PT;
    }
    return engine::audio::Voice::ASTERIA_EN;
}

const char* TTSLocalizationSync::languageForLocale(const std::string& locale) {
    if (locale.size() >= 2) {
        char a = static_cast<char>(std::tolower(static_cast<unsigned char>(locale[0])));
        char b = static_cast<char>(std::tolower(static_cast<unsigned char>(locale[1])));
        if (a == 'p' && b == 't')
            return engine::audio::Lang::PT;
    }
    return engine::audio::Lang::EN;
}

}  // namespace audio
}  // namespace app
