#include "Localizer.hpp"
#include <algorithm>
#include <cctype>

namespace app {
namespace localization {

Localizer::Localizer() {
    setLocale(LANG_EN);
}

std::string Localizer::t(const std::string& key) const {
    auto it = translations_.find(key);
    if (it != translations_.end()) return it->second;
    return key;
}

void Localizer::setLocale(const std::string& locale) {
    std::string normalized = locale;
    if (normalized.size() >= 2) {
        normalized = normalized.substr(0, 2);
        for (auto& c : normalized) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    locale_ = (normalized == "pt") ? LANG_PT : LANG_EN;
    loadTranslations();
}

std::string Localizer::resolveFromSystem(const std::string& systemLocale) {
    if (systemLocale.empty()) return LANG_EN;

    std::string s = systemLocale;
    for (auto& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));

    if (s.size() >= 2) {
        std::string lang = s.substr(0, 2);
        if (lang == "pt") return LANG_PT;
    }
    return LANG_EN;
}

void Localizer::setLocaleFromSystem(const std::string& systemLocale) {
    setLocale(resolveFromSystem(systemLocale));
}

void Localizer::loadTranslations() {
    if (locale_ == LANG_PT) {
        translations_ = translationsPt();
    } else {
        translations_ = translationsEn();
    }
}

}  // namespace localization
}  // namespace app
