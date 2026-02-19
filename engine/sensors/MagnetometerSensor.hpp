#pragma once

#include "ISensor.hpp"

namespace engine {
namespace sensors {

/** Raw magnetometer data (µT) from platform layer. No smoothing applied. */
struct MagnetometerRawData {
    double mx{0.0};  ///< X-axis (µT)
    double my{0.0};  ///< Y-axis (µT)
    double mz{0.0};  ///< Z-axis (µT)
    double timestamp{0.0};
};

/**
 * Magnetometer sensor—raw passthrough only.
 * Platform layer injects data via setRawData(); update() passes through.
 */
class MagnetometerSensor : public ISensor {
public:
    void update(double timestamp) override;

    /** Inject raw magnetometer data from platform layer. */
    void setRawData(const MagnetometerRawData& data);
    void setRawData(double mx, double my, double mz, double timestamp);

    const MagnetometerRawData& getRawData() const { return raw_; }

private:
    MagnetometerRawData raw_;
};

}  // namespace sensors
}  // namespace engine
