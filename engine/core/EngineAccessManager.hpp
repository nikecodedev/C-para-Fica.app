#pragma once

#include "TrackingEngine.hpp"

namespace engine {
namespace core {

/**
 * Feature gate for the tracking engine. Platform layer provides subscription status.
 * No billing logic here—platform sets subscriptionActive based on its billing module.
 *
 * - subscriptionActive == true  → 100Hz full fusion (IMU + GPS + mag)
 * - subscriptionActive == false → 1Hz GPS-only mode (position from latest GPS only)
 */
class EngineAccessManager {
public:
    EngineAccessManager();

    /** Platform calls this with subscription status from billing layer. */
    void setSubscriptionActive(bool active) { subscriptionActive_ = active; }
    bool isSubscriptionActive() const { return subscriptionActive_; }

    /** Access engine for config, sensor push, origin. Platform pushes via getEngine().push*(). */
    TrackingEngine& getEngine() { return engine_; }
    const TrackingEngine& getEngine() const { return engine_; }

    /**
     * Tick with feature gating. Platform calls at 100Hz.
     * When subscribed: full 100Hz fusion.
     * When not: 1Hz GPS-only, returns cached state between updates.
     */
    TrackingState tick(double timestamp);

private:
    TrackingEngine engine_;
    bool subscriptionActive_{false};

    double lastGpsOnlyTickTime_{-1e9};
    TrackingState cachedGpsOnlyState_;
};

}  // namespace core
}  // namespace engine
