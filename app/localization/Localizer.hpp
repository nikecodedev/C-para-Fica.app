#pragma once

#include "LocalizationKeys.hpp"
#include "Translations.hpp"
#include <string>

namespace app {
namespace localization {

/**
 * Key-based string lookup. All UI text uses keys, never hardcoded strings.
 * Supports Portuguese and English with automatic region detection.
 */
class Localizer {
public:
    /** Supported languages (USA, UK, Spain, Portugal, Germany) */
    static constexpr const char* LANG_EN = "en";
    static constexpr const char* LANG_PT = "pt";
    static constexpr const char* LANG_ES = "es";
    static constexpr const char* LANG_DE = "de";

    Localizer();

    /**
     * Get translated string for key. Returns key if translation missing.
     */
    std::string t(const std::string& key) const;

    /** Short alias for t() */
    std::string operator()(const std::string& key) const { return t(key); }

    /** Set language explicitly. Use "en" or "pt". */
    void setLocale(const std::string& locale);

    /** Current locale */
    const std::string& getLocale() const { return locale_; }

    /**
     * Resolve system locale to supported language.
     * Platform passes system locale (e.g. "pt-BR", "en-US", "pt", "en").
     * Returns "pt" or "en"; defaults to "en" if unknown.
     */
    static std::string resolveFromSystem(const std::string& systemLocale);

    /**
     * Detect and set locale from system. Platform calls with system locale string.
     */
    void setLocaleFromSystem(const std::string& systemLocale);

private:
    void loadTranslations();

    std::string locale_{LANG_EN};
    std::unordered_map<std::string, std::string> translations_;
};

}  // namespace localization
}  // namespace app
