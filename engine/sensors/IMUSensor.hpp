#pragma once

#include "ISensor.hpp"

namespace engine {
namespace sensors {

/** Raw accelerometer data (m/s²). */
struct AccelRawData {
    double ax{0.0};
    double ay{0.0};
    double az{0.0};
};

/** Raw gyroscope data (rad/s). */
struct GyroRawData {
    double gx{0.0};
    double gy{0.0};
    double gz{0.0};
};

/** Combined IMU raw data from platform layer. No smoothing applied. */
struct IMURawData {
    AccelRawData accel;
    GyroRawData gyro;
    double timestamp{0.0};
};

/**
 * IMU sensor (accelerometer + gyroscope)—raw passthrough only.
 * Platform layer injects data via setRawData(); update() passes through.
 */
class IMUSensor : public ISensor {
public:
    void update(double timestamp) override;

    /** Inject raw IMU data from platform layer. */
    void setRawData(const IMURawData& data);
    void setRawData(const AccelRawData& accel, const GyroRawData& gyro, double timestamp);

    const IMURawData& getRawData() const { return raw_; }

private:
    IMURawData raw_;
};

}  // namespace sensors
}  // namespace engine
