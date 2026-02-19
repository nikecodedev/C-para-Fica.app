#include "Simulator.hpp"
#include "../math/CoordinateConverter.hpp"
#include "../math/Quaternion.hpp"

namespace engine {
namespace tests {

Simulator::Simulator() = default;

void Simulator::setTrajectory(double vx, double vy, double vz) {
    vx_ = vx;
    vy_ = vy;
    vz_ = vz;
}

void Simulator::setOrigin(double lat_deg, double lon_deg, double alt_m) {
    lat0_ = math::CoordinateConverter::degToRad(lat_deg);
    lon0_ = math::CoordinateConverter::degToRad(lon_deg);
    alt0_ = alt_m;
}

void Simulator::step(double dt, core::TrackingEngine& engine) {
    px_ += vx_ * dt;
    py_ += vy_ * dt;
    pz_ += vz_ * dt;

    math::CoordinateConverter conv;
    conv.setOrigin(lat0_, lon0_, alt0_);
    math::Geodetic g = conv.enuToGeodetic(px_, py_, pz_);

    double lat_deg = math::CoordinateConverter::radToDeg(g.latitude);
    double lon_deg = math::CoordinateConverter::radToDeg(g.longitude);
    engine.pushGps(t_, lat_deg, lon_deg, g.altitude, 1.0);

    double ax = 0, ay = 0, az = 9.81;
    double gx = 0.001 * std::sin(t_);
    double gy = 0.001 * std::cos(t_);
    double gz = 0;
    engine.pushImu(t_, ax, ay, az, gx, gy, gz);

    engine.pushMag(t_, 0.0, 1.0, 0.0);

    t_ += dt;
}

}  // namespace tests
}  // namespace engine
