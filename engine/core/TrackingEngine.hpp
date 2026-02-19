#pragma once

#include "RingBuffer.hpp"
#include "../fusion/ExtendedKalmanFilter.hpp"
#include "../fusion/EKFConfig.hpp"
#include "../math/CoordinateConverter.hpp"
#include "../math/Quaternion.hpp"

namespace engine {
namespace core {

/** Timestamped IMU sample. */
struct IMUSample {
    double timestamp{0.0};
    double ax{0.0}, ay{0.0}, az{0.0};  ///< m/s²
    double gx{0.0}, gy{0.0}, gz{0.0};  ///< rad/s
};

/** Timestamped GPS sample (degrees for lat/lon). */
struct GPSSample {
    double timestamp{0.0};
    double lat_deg{0.0}, lon_deg{0.0}, alt_m{0.0};
    double accuracy_m{0.0};
};

/** Timestamped magnetometer sample (µT). */
struct MagSample {
    double timestamp{0.0};
    double mx{0.0}, my{0.0}, mz{0.0};
};

/** Fused output at 100Hz. */
struct TrackingState {
    double timestamp{0.0};
    double px{0.0}, py{0.0}, pz{0.0};       ///< ENU position (m)
    double vx{0.0}, vy{0.0}, vz{0.0};       ///< ENU velocity (m/s)
    double qw{1.0}, qx{0.0}, qy{0.0}, qz{0.0};  ///< Orientation
};

/** Buffer capacities (tuned for ~1s of 100Hz IMU, 2s GPS, 1s mag). */
constexpr size_t IMU_BUFFER_SIZE = 128;
constexpr size_t GPS_BUFFER_SIZE = 16;
constexpr size_t MAG_BUFFER_SIZE = 64;

/**
 * 100Hz tracking engine. Deterministic, no dynamic alloc, non-blocking.
 * Platform layer must call tick() at 100Hz (every 10ms).
 */
class TrackingEngine {
public:
    TrackingEngine();

    /** Config and origin must be set before first tick. */
    void setConfig(const fusion::EKFConfig& config);
    void setOrigin(double lat_deg, double lon_deg, double alt_m);
    void setOrigin(double lat_rad, double lon_rad, double alt_m, bool radians);

    /** Push sensor data. Non-blocking, overwrites oldest if buffer full. */
    void pushImu(double timestamp, double ax, double ay, double az, double gx, double gy, double gz);
    void pushGps(double timestamp, double lat_deg, double lon_deg, double alt_m, double accuracy_m = 10.0);
    void pushMag(double timestamp, double mx, double my, double mz);

    /**
     * Process one 100Hz tick. Call at fixed 10ms intervals.
     * @param timestamp Current time (s). Must be monotonic.
     * @return Fused state; invalid if no IMU yet.
     */
    TrackingState tick(double timestamp);

    /** Whether engine has enough data to produce valid output. */
    bool isReady() const { return hasImu_; }

    fusion::ExtendedKalmanFilter& getEKF() { return ekf_; }
    const fusion::ExtendedKalmanFilter& getEKF() const { return ekf_; }

    math::CoordinateConverter& getCoordinateConverter() { return converter_; }

private:
    /** Interpolate IMU at target timestamp. Returns false if no data. */
    bool getImuAt(double t, IMUSample& out) const;

    /** Latest GPS with timestamp <= t. Returns false if none. */
    bool getGpsAt(double t, GPSSample& out) const;

    /** Latest mag with timestamp <= t. Returns false if none. */
    bool getMagAt(double t, MagSample& out) const;

    fusion::ExtendedKalmanFilter ekf_;
    math::CoordinateConverter converter_;

    RingBuffer<IMUSample, IMU_BUFFER_SIZE> imuBuffer_;
    RingBuffer<GPSSample, GPS_BUFFER_SIZE> gpsBuffer_;
    RingBuffer<MagSample, MAG_BUFFER_SIZE> magBuffer_;

    double lastTickTime_{0.0};
    bool lastTickTimeValid_{false};
    bool hasImu_{false};
    bool originSet_{false};

    double lastAppliedGpsTime_{-1e9};  ///< Avoid double-applying same GPS
    double lastAppliedMagTime_{-1e9};
};

}  // namespace core
}  // namespace engine
