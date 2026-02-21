#pragma once

#include "TrackingEngine.hpp"

namespace engine {
namespace core {

/**
 * Feature gate for the tracking engine. Platform provides subscription and optional
 * "Chave com Validade" (license key) for client tests.
 *
 * Full fusion when: subscriptionActive_ || licenseKeyActive_.
 * Phase 1: fusion runs at 20Hz (stable); EKF holds track during GPS loss.
 * When not active: 1Hz GPS-only.
 */
class EngineAccessManager {
public:
    EngineAccessManager();

    /** Platform calls with subscription status from billing. */
    void setSubscriptionActive(bool active) { subscriptionActive_ = active; }
    bool isSubscriptionActive() const { return subscriptionActive_; }

    /** Chave com Validade: when true, full fusion enabled for client tests/homologation. */
    void setLicenseKeyActive(bool active) { licenseKeyActive_ = active; }
    bool isLicenseKeyActive() const { return licenseKeyActive_; }

    /** True when full fusion is enabled (subscription or valid license key). */
    bool isFullFusionEnabled() const { return subscriptionActive_ || licenseKeyActive_; }

    /** Access engine for config, sensor push, origin. */
    TrackingEngine& getEngine() { return engine_; }
    const TrackingEngine& getEngine() const { return engine_; }

    /**
     * Tick with feature gating. Platform can call at 100Hz for UI.
     * When full fusion enabled: runs at 20Hz (phase 1 stability), returns cached state between.
     * When not: 1Hz GPS-only.
     */
    TrackingState tick(double timestamp);

private:
    TrackingEngine engine_;
    bool subscriptionActive_{false};
    bool licenseKeyActive_{false};

    /** Phase 1: 20Hz fusion for stability; EKF holds track on signal loss. */
    static constexpr double FUSION_PERIOD_PHASE1 = 0.05;  // 20 Hz
    double lastFusionTickTime_{-1e9};
    TrackingState cachedFusionState_;

    double lastGpsOnlyTickTime_{-1e9};
    TrackingState cachedGpsOnlyState_;
};

}  // namespace core
}  // namespace engine
