#pragma once

#include <array>
#include <cstddef>

namespace engine {
namespace logging {

/**
 * Single log entry for tuning. Fixed layout—no dynamic allocation.
 */
struct TrackingLogEntry {
    double timestamp{0.0};

    /* Fused state (ENU) */
    double px{0.0}, py{0.0}, pz{0.0};
    double vx{0.0}, vy{0.0}, vz{0.0};
    double qw{1.0}, qx{0.0}, qy{0.0}, qz{0.0};

    /* Covariance diagonal (16) */
    double P[16]{0};

    /* Raw sensor data used this tick */
    double raw_ax{0.0}, raw_ay{0.0}, raw_az{0.0};
    double raw_gx{0.0}, raw_gy{0.0}, raw_gz{0.0};
    bool has_raw_imu{false};

    double raw_gps_x{0.0}, raw_gps_y{0.0}, raw_gps_z{0.0};
    bool has_raw_gps{false};

    double raw_mx{0.0}, raw_my{0.0}, raw_mz{0.0};
    bool has_raw_mag{false};
};

/** ~10 minutes at 100Hz */
constexpr size_t LOG_BUFFER_SIZE = 60000;

/**
 * Tuning logger. Non-blocking log(), export to CSV on demand.
 * No dynamic allocation in log()—safe for 100Hz loop.
 *
 * CSV columns: timestamp | fused state (px..qz) | covariance diag (P0..P15) |
 *              raw IMU (ax,ay,az,gx,gy,gz) | raw GPS ENU (x,y,z) | raw mag (mx,my,mz)
 * has_raw_* flags indicate sensor data availability that tick.
 */
class TrackingLogger {
public:
    TrackingLogger();

    /**
     * Log one tick. Non-blocking, no allocation. Overwrites oldest if full.
     */
    void log(const TrackingLogEntry& entry);

    /** Export buffer to CSV. Call off the hot path. */
    bool exportToCsv(const char* path) const;

    /** Clear buffer. */
    void clear();

    size_t size() const { return size_; }
    bool empty() const { return size_ == 0; }

private:
    std::array<TrackingLogEntry, LOG_BUFFER_SIZE> data_;
    size_t head_{0};
    size_t size_{0};
};

}  // namespace logging
}  // namespace engine
