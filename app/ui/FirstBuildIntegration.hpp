#pragma once

#include "../../engine/core/EngineAccessManager.hpp"
#include "../../engine/core/TrackingEngine.hpp"
#include "../../engine/config/UnitConverter.hpp"
#include "../../engine/logging/TrackingLogger.hpp"
#include "audio/AudioTrackingIntegration.hpp"
#include <vector>
#include <cmath>

namespace app {
namespace ui {

/**
 * First build integration. Wires engine, EKF, logging, 100Hz.
 * Provides display values: speed, distance, trajectory.
 * Do NOT attempt full tuning—use defaults.
 */
class FirstBuildIntegration {
public:
    FirstBuildIntegration();

    engine::core::EngineAccessManager& getEngine() { return engine_; }
    const engine::core::EngineAccessManager& getEngine() const { return engine_; }

    /**
     * Call at 100Hz from platform timer.
     * Drains voice commands first (non-blocking) so audio never blocks the loop.
     */
    void tick(double timestamp);

    /** Optional: set to drain voice commands at start of each tick. */
    void setAudioIntegration(app::audio::AudioTrackingIntegration* integration) {
        audioIntegration_ = integration;
    }

    /** Set subscription (true = full fusion). */
    void setSubscriptionActive(bool active) { engine_.setSubscriptionActive(active); }

    /** Chave com Validade: enable full fusion for client tests (platform sets after validating key). */
    void setLicenseKeyActive(bool active) { engine_.setLicenseKeyActive(active); }

    /** For display: speed in display units (km/h or mph). */
    double getSpeedDisplay() const { return speedDisplay_; }

    /** For display: accumulated distance in display units. */
    double getDistanceDisplay() const { return distanceDisplay_; }

    /** For display: trajectory points (px, py) in ENU meters. */
    const std::vector<std::pair<double, double>>& getTrajectory() const { return trajectory_; }

    /** Last fused state from most recent tick (for JNI out array). */
    const engine::core::TrackingState& getLastState() const { return lastState_; }

    /** Unit labels. */
    const char* getSpeedUnit() const { return unitConv_.speedUnitLabel(); }
    const char* getDistanceUnit() const { return unitConv_.distanceUnitLabel(); }

    /** Set unit system from region. */
    void setUnitSystemFromRegion(const char* region) { unitConv_.setFromRegion(region); }

    /** Export log when session ends. */
    bool exportLog(const char* path) const { return logger_.exportToCsv(path); }

    /** Reset accumulated distance and trajectory. */
    void resetSession();

private:
    engine::core::EngineAccessManager engine_;
    engine::config::UnitConverter unitConv_;
    engine::logging::TrackingLogger logger_;

    double speedDisplay_{0};
    double distanceDisplay_{0};
    engine::core::TrackingState lastState_{};
    double lastPx_{0}, lastPy_{0}, lastPz_{0};
    bool hasLastPos_{false};

    std::vector<std::pair<double, double>> trajectory_;
    app::audio::AudioTrackingIntegration* audioIntegration_{nullptr};
};

}  // namespace ui
}  // namespace app
