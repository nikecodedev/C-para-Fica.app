#pragma once

#include "../core/TrackingEngine.hpp"
#include <cmath>
#include <cstdlib>

namespace engine {
namespace tests {

/**
 * Simulation mode: generate synthetic sensor data for debugging.
 * Deterministic trajectory. Use for fusion validation without field tests.
 */
class Simulator {
public:
    Simulator();

    void setTrajectory(double vx, double vy, double vz);
    void setOrigin(double lat_deg, double lon_deg, double alt_m);
    void step(double dt, core::TrackingEngine& engine);

    double getTime() const { return t_; }

private:
    double t_{0};
    double px_{0}, py_{0}, pz_{0};
    double vx_{0}, vy_{0}, vz_{0};
    double lat0_{0}, lon0_{0}, alt0_{0};
};

}  // namespace tests
}  // namespace engine
