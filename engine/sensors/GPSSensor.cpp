#include "GPSSensor.hpp"

namespace engine {
namespace sensors {

void GPSSensor::update(double timestamp) {
    (void)timestamp;
    /* Raw passthrough: no smoothing, no filtering. */
}

void GPSSensor::setRawData(const GPSRawData& data) {
    raw_ = data;
}

void GPSSensor::setRawData(double lat, double lon, double alt, double accuracy,
                           double speed, double course, double timestamp) {
    raw_.latitude = lat;
    raw_.longitude = lon;
    raw_.altitude = alt;
    raw_.accuracy = accuracy;
    raw_.speed = speed;
    raw_.course = course;
    raw_.timestamp = timestamp;
}

}  // namespace sensors
}  // namespace engine
