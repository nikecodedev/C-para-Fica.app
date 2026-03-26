#include "app/ui/FirstBuildIntegration.hpp"

namespace app {
namespace ui {

FirstBuildIntegration::FirstBuildIntegration() {
    engine_.getEngine().setLogger(&logger_);
}

void FirstBuildIntegration::tick(double timestamp) {
    if (audioIntegration_)
        audioIntegration_->drainVoiceCommands();
    lastState_ = engine_.tick(timestamp);
    auto& state = lastState_;

    double v = std::sqrt(state.vx * state.vx + state.vy * state.vy + state.vz * state.vz);
    speedDisplay_ = unitConv_.speedToDisplay(v);

    if (hasLastPos_) {
        double dx = state.px - lastPx_;
        double dy = state.py - lastPy_;
        double dz = state.pz - lastPz_;
        double step = std::sqrt(dx*dx + dy*dy + dz*dz);
        // Only accumulate distance when moving; ignore GPS/IMU drift when stationary
        constexpr double MIN_SPEED_MS = 0.15;  // ~0.5 km/h
        if (v >= MIN_SPEED_MS) {
            distanceDisplay_ += unitConv_.distanceToDisplay(step);
        }
    }
    lastPx_ = state.px;
    lastPy_ = state.py;
    lastPz_ = state.pz;
    hasLastPos_ = true;

    trajectory_.emplace_back(state.px, state.py);
}

void FirstBuildIntegration::resetSession() {
    distanceDisplay_ = 0;
    trajectory_.clear();
    hasLastPos_ = false;
}

}  // namespace ui
}  // namespace app
