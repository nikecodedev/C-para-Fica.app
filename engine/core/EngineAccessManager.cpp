#include "EngineAccessManager.hpp"

namespace engine {
namespace core {

namespace {
    constexpr double GPS_ONLY_PERIOD = 1.0;  // 1 Hz
}

EngineAccessManager::EngineAccessManager() {
    cachedGpsOnlyState_.qw = 1.0;
    cachedFusionState_.qw = 1.0;
}

TrackingState EngineAccessManager::tick(double timestamp) {
    const bool fullFusion = subscriptionActive_ || licenseKeyActive_;

    if (fullFusion) {
        if (timestamp - lastFusionTickTime_ >= FUSION_PERIOD_PHASE1) {
            lastFusionTickTime_ = timestamp;
            cachedFusionState_ = engine_.tick(timestamp);
        } else {
            cachedFusionState_.timestamp = timestamp;
        }
        return cachedFusionState_;
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
