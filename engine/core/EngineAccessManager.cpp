#include "EngineAccessManager.hpp"

namespace engine {
namespace core {

namespace {
    constexpr double GPS_ONLY_PERIOD = 1.0;  // 1 Hz
}

EngineAccessManager::EngineAccessManager() {
    cachedGpsOnlyState_.qw = 1.0;
}

TrackingState EngineAccessManager::tick(double timestamp) {
    if (subscriptionActive_) {
        return engine_.tick(timestamp);
    }

    if (timestamp - lastGpsOnlyTickTime_ >= GPS_ONLY_PERIOD) {
        lastGpsOnlyTickTime_ = timestamp;
        cachedGpsOnlyState_ = engine_.tickGpsOnly(timestamp);
    }

    cachedGpsOnlyState_.timestamp = timestamp;
    return cachedGpsOnlyState_;
}

}  // namespace core
}  // namespace engine
