#include "UnitConverter.hpp"
#include <cstring>

namespace engine {
namespace config {

double UnitConverter::distanceToDisplay(double meters) const {
    if (system_ == UnitSystem::Imperial) {
        return meters / M_PER_MILE;
    }
    return meters / M_PER_KM;
}

double UnitConverter::speedToDisplay(double m_per_sec) const {
    if (system_ == UnitSystem::Imperial) {
        return m_per_sec * MS_TO_MPH;
    }
    return m_per_sec * MS_TO_KMH;
}

double UnitConverter::displayToDistance(double display) const {
    if (system_ == UnitSystem::Imperial) {
        return display * M_PER_MILE;
    }
    return display * M_PER_KM;
}

double UnitConverter::displayToSpeed(double display) const {
    if (system_ == UnitSystem::Imperial) {
        return display / MS_TO_MPH;
    }
    return display / MS_TO_KMH;
}

const char* UnitConverter::distanceUnitLabel() const {
    return (system_ == UnitSystem::Imperial) ? "mi" : "km";
}

const char* UnitConverter::speedUnitLabel() const {
    return (system_ == UnitSystem::Imperial) ? "mph" : "km/h";
}

void UnitConverter::setFromRegion(const char* regionCode) {
    if (!regionCode || regionCode[0] == '\0') {
        system_ = UnitSystem::Metric;
        return;
    }
    if (regionCode[0] == 'U' && regionCode[1] == 'S') { system_ = UnitSystem::Imperial; return; }
    if (regionCode[0] == 'U' && regionCode[1] == 'K') { system_ = UnitSystem::Imperial; return; }
    if (regionCode[0] == 'G' && regionCode[1] == 'B') { system_ = UnitSystem::Imperial; return; }
    if (regionCode[0] == 'M' && regionCode[1] == 'M') { system_ = UnitSystem::Imperial; return; }
    system_ = UnitSystem::Metric;
}

}  // namespace config
}  // namespace engine
