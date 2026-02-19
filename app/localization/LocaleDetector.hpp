#pragma once

#include <string>

namespace app {
namespace localization {

/**
 * Platform-provided system locale detection.
 * iOS/Android implement getSystemLocaleImpl() and register it at startup.
 * Engine uses this for automatic language selection.
 */
class LocaleDetector {
public:
    /** Provider signature: returns system locale string (e.g. "pt-BR", "en-US") */
    using Provider = std::string (*)();

    /** Register platform implementation. Call from platform startup. */
    static void setProvider(Provider provider) { provider_ = provider; }

    /**
     * Get system locale. Uses registered provider, or "en" if none.
     */
    static std::string getSystemLocale();

private:
    static Provider provider_;
};

}  // namespace localization
}  // namespace app
