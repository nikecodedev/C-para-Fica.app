#include "MagnetometerSensor.hpp"

namespace engine {
namespace sensors {

void MagnetometerSensor::update(double timestamp) {
    (void)timestamp;
    /* Raw passthrough: no smoothing, no filtering. */
}

void MagnetometerSensor::setRawData(const MagnetometerRawData& data) {
    raw_ = data;
}

void MagnetometerSensor::setRawData(double mx, double my, double mz, double timestamp) {
    raw_.mx = mx;
    raw_.my = my;
    raw_.mz = mz;
    raw_.timestamp = timestamp;
}

}  // namespace sensors
}  // namespace engine
