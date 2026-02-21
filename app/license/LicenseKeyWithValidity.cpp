#include "LicenseKeyWithValidity.hpp"
#include <cstdlib>
#include <cstring>

namespace app {
namespace license {

namespace {
    constexpr const char* PREFIX = "FICA-TEST-";
    constexpr size_t PREFIX_LEN = 10;
}

void LicenseKeyWithValidity::setKey(const std::string& key) {
    key_ = key;
    validityEndUnix_ = 0.0;

    if (key_.size() <= PREFIX_LEN) return;
    if (std::strncmp(key_.c_str(), PREFIX, PREFIX_LEN) != 0) return;

    const char* rest = key_.c_str() + PREFIX_LEN;
    char* end = nullptr;
    double epoch = std::strtod(rest, &end);
    if (end != rest && epoch > 0)
        validityEndUnix_ = epoch;
}

void LicenseKeyWithValidity::clear() {
    key_.clear();
    validityEndUnix_ = 0.0;
}

bool LicenseKeyWithValidity::isValid(double currentTimeUnix) const {
    if (key_.empty()) return false;
    if (validityEndUnix_ <= 0.0) return true;  // key set but no expiry parsed → treat as valid (e.g. dev)
    return currentTimeUnix < validityEndUnix_;
}

}  // namespace license
}  // namespace app
