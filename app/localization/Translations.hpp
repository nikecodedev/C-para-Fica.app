#pragma once

#include <unordered_map>
#include <string>

namespace app {
namespace localization {

/** English translations */
std::unordered_map<std::string, std::string> translationsEn();

/** Portuguese translations */
std::unordered_map<std::string, std::string> translationsPt();

}  // namespace localization
}  // namespace app
