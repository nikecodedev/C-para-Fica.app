#pragma once

#include "ISensor.hpp"

namespace engine {
namespace sensors {

/** Raw GPS data from platform layer. No smoothing applied. */
struct GPSRawData {
    double latitude{0.0};   ///< Degrees
    double longitude{0.0};  ///< Degrees
    double altitude{0.0};   ///< Meters (WGS84)
    double accuracy{0.0};  ///< Meters (horizontal accuracy)
    double speed{0.0};     ///< m/s
    double course{0.0};    ///< Degrees (0-360, true north)
    double timestamp{0.0};
};

/**
 * GPS sensor—raw passthrough only.
 * Platform layer injects data via setRawData(); update() passes through.
 */
class GPSSensor : public ISensor {
public:
    void update(double timestamp) override;

    /** Inject raw GPS data from platform layer. */
    void setRawData(const GPSRawData& data);
    void setRawData(double lat, double lon, double alt, double accuracy,
                    double speed, double course, double timestamp);

    const GPSRawData& getRawData() const { return raw_; }

private:
    GPSRawData raw_;
};

}  // namespace sensors
}  // namespace engine
