#include "IMUSensor.hpp"

namespace engine {
namespace sensors {

void IMUSensor::update(double timestamp) {
    (void)timestamp;
    /* Raw passthrough: no smoothing, no filtering. */
}

void IMUSensor::setRawData(const IMURawData& data) {
    raw_ = data;
}

void IMUSensor::setRawData(const AccelRawData& accel, const GyroRawData& gyro, double timestamp) {
    raw_.accel = accel;
    raw_.gyro = gyro;
    raw_.timestamp = timestamp;
}

}  // namespace sensors
}  // namespace engine
