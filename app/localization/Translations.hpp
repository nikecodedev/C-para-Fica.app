#pragma once

#include <unordered_map>
#include <string>

namespace app {
namespace localization {

/** English translations (US, UK) */
std::unordered_map<std::string, std::string> translationsEn();

/** Portuguese translations (PT, BR) */
std::unordered_map<std::string, std::string> translationsPt();

/** Spanish translations (ES) */
std::unordered_map<std::string, std::string> translationsEs();

/** German translations (DE) */
std::unordered_map<std::string, std::string> translationsDe();

}  // namespace localization
}  // namespace app
