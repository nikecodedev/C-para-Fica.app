#include "TrackingEngine.hpp"
#include "../logging/TrackingLogger.hpp"
#include "../math/CoordinateConverter.hpp"
#include <cmath>

namespace engine {
namespace core {

namespace {
    constexpr double TICK_PERIOD = 0.01;  // 100 Hz
}

TrackingEngine::TrackingEngine() {
    fusion::EKFConfig config;
    ekf_.setConfig(config);
}

void TrackingEngine::setConfig(const fusion::EKFConfig& config) {
    ekf_.setConfig(config);
}

void TrackingEngine::setOrigin(double lat_deg, double lon_deg, double alt_m) {
    const double lat_rad = math::CoordinateConverter::degToRad(lat_deg);
    const double lon_rad = math::CoordinateConverter::degToRad(lon_deg);
    converter_.setOrigin(lat_rad, lon_rad, alt_m);
    originSet_ = true;
}

void TrackingEngine::setOrigin(double lat_rad, double lon_rad, double alt_m, bool /*radians*/) {
    converter_.setOrigin(lat_rad, lon_rad, alt_m);
    originSet_ = true;
}

void TrackingEngine::pushImu(double timestamp, double ax, double ay, double az, double gx, double gy, double gz) {
    imuBuffer_.push(IMUSample{timestamp, ax, ay, az, gx, gy, gz});
}

void TrackingEngine::pushGps(double timestamp, double lat_deg, double lon_deg, double alt_m, double accuracy_m) {
    gpsBuffer_.push(GPSSample{timestamp, lat_deg, lon_deg, alt_m, accuracy_m});
}

void TrackingEngine::pushMag(double timestamp, double mx, double my, double mz) {
    magBuffer_.push(MagSample{timestamp, mx, my, mz});
}

bool TrackingEngine::getImuAt(double t, IMUSample& out) const {
    if (imuBuffer_.empty()) return false;

    const size_t n = imuBuffer_.size();
    if (t <= imuBuffer_[0].timestamp) {
        out = imuBuffer_[0];
        return true;
    }
    if (t >= imuBuffer_[n - 1].timestamp) {
        out = imuBuffer_[n - 1];
        return true;
    }

    size_t hi = 1;
    while (hi < n && imuBuffer_[hi].timestamp < t) ++hi;
    const size_t lo = hi - 1;

    const IMUSample& a = imuBuffer_[lo];
    const IMUSample& b = imuBuffer_[hi];
    const double denom = b.timestamp - a.timestamp;
    const double frac = (denom > 1e-9) ? ((t - a.timestamp) / denom) : 0.0;

    out.timestamp = t;
    out.ax = a.ax + frac * (b.ax - a.ax);
    out.ay = a.ay + frac * (b.ay - a.ay);
    out.az = a.az + frac * (b.az - a.az);
    out.gx = a.gx + frac * (b.gx - a.gx);
    out.gy = a.gy + frac * (b.gy - a.gy);
    out.gz = a.gz + frac * (b.gz - a.gz);
    return true;
}

bool TrackingEngine::getGpsAt(double t, GPSSample& out) const {
    if (gpsBuffer_.empty()) return false;

    for (size_t i = gpsBuffer_.size(); i > 0; --i) {
        const GPSSample& s = gpsBuffer_[i - 1];
        if (s.timestamp <= t) {
            out = s;
            return true;
        }
    }
    return false;
}

bool TrackingEngine::getMagAt(double t, MagSample& out) const {
    if (magBuffer_.empty()) return false;

    for (size_t i = magBuffer_.size(); i > 0; --i) {
        const MagSample& s = magBuffer_[i - 1];
        if (s.timestamp <= t) {
            out = s;
            return true;
        }
    }
    return false;
}

TrackingState TrackingEngine::tick(double timestamp) {
    TrackingState state;
    state.timestamp = timestamp;

    double dt = TICK_PERIOD;
    if (lastTickTimeValid_) {
        dt = timestamp - lastTickTime_;
        if (dt <= 0.0 || dt > 0.5) dt = TICK_PERIOD;
    }
    lastTickTime_ = timestamp;
    lastTickTimeValid_ = true;

    IMUSample imu;
    bool hasImuThisTick = getImuAt(timestamp, imu);
    if (hasImuThisTick) {
        hasImu_ = true;
        ekf_.setImuInput(imu.ax, imu.ay, imu.az, imu.gx, imu.gy, imu.gz);
    }

    ekf_.predict(dt);

    double gpsE[3] = {0, 0, 0};
    bool hasGpsThisTick = false;
    if (originSet_ || !gpsBuffer_.empty()) {
        GPSSample gps;
        if (getGpsAt(timestamp, gps)) {
            if (!originSet_) {
                setOrigin(gps.lat_deg, gps.lon_deg, gps.alt_m);
            }
            const double lat_rad = math::CoordinateConverter::degToRad(gps.lat_deg);
            const double lon_rad = math::CoordinateConverter::degToRad(gps.lon_deg);
            math::ENU enu = converter_.geodeticToENU(lat_rad, lon_rad, gps.alt_m);
            gpsE[0] = enu.east;
            gpsE[1] = enu.north;
            gpsE[2] = enu.up;
            hasGpsThisTick = true;
            if (gps.timestamp > lastAppliedGpsTime_) {
                lastAppliedGpsTime_ = gps.timestamp;
                ekf_.updateGPS(enu.east, enu.north, enu.up);
            }
        }
    }

    double magV[3] = {0, 0, 0};
    bool hasMagThisTick = false;
    MagSample mag;
    if (getMagAt(timestamp, mag)) {
        magV[0] = mag.mx;
        magV[1] = mag.my;
        magV[2] = mag.mz;
        hasMagThisTick = true;
        if (mag.timestamp > lastAppliedMagTime_) {
            lastAppliedMagTime_ = mag.timestamp;
            ekf_.updateMagnetometer(mag.mx, mag.my, mag.mz);
        }
    }

    const auto& x = ekf_.getState();
    state.px = x(fusion::StateIndex::PX, 0);
    state.py = x(fusion::StateIndex::PY, 0);
    state.pz = x(fusion::StateIndex::PZ, 0);
    state.vx = x(fusion::StateIndex::VX, 0);
    state.vy = x(fusion::StateIndex::VY, 0);
    state.vz = x(fusion::StateIndex::VZ, 0);
    state.qw = x(fusion::StateIndex::QW, 0);
    state.qx = x(fusion::StateIndex::QX, 0);
    state.qy = x(fusion::StateIndex::QY, 0);
    state.qz = x(fusion::StateIndex::QZ, 0);

    if (logger_) {
        logging::TrackingLogEntry entry;
        entry.timestamp = timestamp;
        entry.px = state.px;
        entry.py = state.py;
        entry.pz = state.pz;
        entry.vx = state.vx;
        entry.vy = state.vy;
        entry.vz = state.vz;
        entry.qw = state.qw;
        entry.qx = state.qx;
        entry.qy = state.qy;
        entry.qz = state.qz;
        const auto& P = ekf_.getCovariance();
        for (int i = 0; i < 16; ++i) entry.P[i] = P(i, i);
        if (hasImuThisTick) {
            entry.raw_ax = imu.ax;
            entry.raw_ay = imu.ay;
            entry.raw_az = imu.az;
            entry.raw_gx = imu.gx;
            entry.raw_gy = imu.gy;
            entry.raw_gz = imu.gz;
            entry.has_raw_imu = true;
        }
        if (hasGpsThisTick) {
            entry.raw_gps_x = gpsE[0];
            entry.raw_gps_y = gpsE[1];
            entry.raw_gps_z = gpsE[2];
            entry.has_raw_gps = true;
        }
        if (hasMagThisTick) {
            entry.raw_mx = magV[0];
            entry.raw_my = magV[1];
            entry.raw_mz = magV[2];
            entry.has_raw_mag = true;
        }
        logger_->log(entry);
    }

    return state;
}

TrackingState TrackingEngine::tickGpsOnly(double timestamp) {
    TrackingState state;
    state.timestamp = timestamp;
    state.px = state.py = state.pz = 0.0;
    state.vx = state.vy = state.vz = 0.0;
    state.qw = 1.0;
    state.qx = state.qy = state.qz = 0.0;

    GPSSample gps;
    if (!getGpsAt(timestamp, gps)) return state;

    if (!originSet_) {
        setOrigin(gps.lat_deg, gps.lon_deg, gps.alt_m);
    }
    const double lat_rad = math::CoordinateConverter::degToRad(gps.lat_deg);
    const double lon_rad = math::CoordinateConverter::degToRad(gps.lon_deg);
    math::ENU enu = converter_.geodeticToENU(lat_rad, lon_rad, gps.alt_m);

    state.px = enu.east;
    state.py = enu.north;
    state.pz = enu.up;

    return state;
}

}  // namespace core
}  // namespace engine
