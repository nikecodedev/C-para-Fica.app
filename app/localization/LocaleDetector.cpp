#include "LocaleDetector.hpp"

namespace app {
namespace localization {

LocaleDetector::Provider LocaleDetector::provider_ = nullptr;

std::string LocaleDetector::getSystemLocale() {
    if (provider_) return provider_();
    return "en";
}

}  // namespace localization
}  // namespace app
