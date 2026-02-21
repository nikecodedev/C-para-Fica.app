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
    if (normalized == "pt") locale_ = LANG_PT;
    else if (normalized == "es") locale_ = LANG_ES;
    else if (normalized == "de") locale_ = LANG_DE;
    else locale_ = LANG_EN;
    loadTranslations();
}

std::string Localizer::resolveFromSystem(const std::string& systemLocale) {
    if (systemLocale.empty()) return LANG_EN;

    std::string s = systemLocale;
    for (auto& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));

    if (s.size() >= 2) {
        std::string lang = s.substr(0, 2);
        if (lang == "pt") return LANG_PT;  // Portugal, Brazil
        if (lang == "es") return LANG_ES;  // Spain
        if (lang == "de") return LANG_DE;  // Germany
    }
    return LANG_EN;  // USA, UK, default
}

void Localizer::setLocaleFromSystem(const std::string& systemLocale) {
    setLocale(resolveFromSystem(systemLocale));
}

void Localizer::loadTranslations() {
    if (locale_ == LANG_PT) translations_ = translationsPt();
    else if (locale_ == LANG_ES) translations_ = translationsEs();
    else if (locale_ == LANG_DE) translations_ = translationsDe();
    else translations_ = translationsEn();
}

}  // namespace localization
}  // namespace app
