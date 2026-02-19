#include "CoordinateConverter.hpp"

namespace engine {
namespace math {

namespace {

/** Radius of curvature in prime vertical: N = a / sqrt(1 - e²*sin²(lat)) */
inline double primeVerticalRadius(double lat_rad) {
    const double sinLat = std::sin(lat_rad);
    return WGS84::A / std::sqrt(1.0 - WGS84::E2 * sinLat * sinLat);
}

}  // namespace

// -----------------------------------------------------------------------------
// Origin
// -----------------------------------------------------------------------------

void CoordinateConverter::setOrigin(double lat_rad, double lon_rad, double alt_m) {
    origin_.latitude = lat_rad;
    origin_.longitude = lon_rad;
    origin_.altitude = alt_m;
    originSet_ = true;
    updateOriginECEF();
}

void CoordinateConverter::setOrigin(const Geodetic& origin) {
    origin_ = origin;
    originSet_ = true;
    updateOriginECEF();
}

void CoordinateConverter::updateOriginECEF() {
    originECEF_ = geodeticToECEF(origin_.latitude, origin_.longitude, origin_.altitude);
}

// -----------------------------------------------------------------------------
// Geodetic ↔ ECEF
// -----------------------------------------------------------------------------

ECEF CoordinateConverter::geodeticToECEF(double lat_rad, double lon_rad, double alt_m) const {
    const double N = primeVerticalRadius(lat_rad);
    const double sinLat = std::sin(lat_rad);
    const double cosLat = std::cos(lat_rad);
    const double sinLon = std::sin(lon_rad);
    const double cosLon = std::cos(lon_rad);

    ECEF e;
    e.x = (N + alt_m) * cosLat * cosLon;
    e.y = (N + alt_m) * cosLat * sinLon;
    e.z = (N * (1.0 - WGS84::E2) + alt_m) * sinLat;
    return e;
}

ECEF CoordinateConverter::geodeticToECEF(const Geodetic& g) const {
    return geodeticToECEF(g.latitude, g.longitude, g.altitude);
}

Geodetic CoordinateConverter::ecefToGeodetic(double x, double y, double z) const {
    const double p = std::sqrt(x * x + y * y);
    const double lon = std::atan2(y, x);

    if (p < 1e-12) {
        Geodetic g;
        g.longitude = lon;
        g.latitude = (z >= 0) ? (3.14159265358979323846 / 2.0) : (-3.14159265358979323846 / 2.0);
        g.altitude = std::abs(z) - WGS84::A * std::sqrt(1.0 - WGS84::E2);
        return g;
    }

    double lat = std::atan2(z, p * (1.0 - WGS84::E2));
    for (int i = 0; i < 5; ++i) {
        const double N = primeVerticalRadius(lat);
        lat = std::atan2(z + WGS84::E2 * N * std::sin(lat), p);
    }

    const double N = primeVerticalRadius(lat);
    const double alt = p / std::cos(lat) - N;

    Geodetic g;
    g.latitude = lat;
    g.longitude = lon;
    g.altitude = alt;
    return g;
}

Geodetic CoordinateConverter::ecefToGeodetic(const ECEF& e) const {
    return ecefToGeodetic(e.x, e.y, e.z);
}

// -----------------------------------------------------------------------------
// ECEF ↔ ENU
// -----------------------------------------------------------------------------

ENU CoordinateConverter::ecefToENU(double x, double y, double z) const {
    const double dx = x - originECEF_.x;
    const double dy = y - originECEF_.y;
    const double dz = z - originECEF_.z;

    const double sinLat = std::sin(origin_.latitude);
    const double cosLat = std::cos(origin_.latitude);
    const double sinLon = std::sin(origin_.longitude);
    const double cosLon = std::cos(origin_.longitude);

    ENU e;
    e.east = -sinLon * dx + cosLon * dy;
    e.north = -sinLat * cosLon * dx - sinLat * sinLon * dy + cosLat * dz;
    e.up = cosLat * cosLon * dx + cosLat * sinLon * dy + sinLat * dz;
    return e;
}

ENU CoordinateConverter::ecefToENU(const ECEF& e) const {
    return ecefToENU(e.x, e.y, e.z);
}

ECEF CoordinateConverter::enuToECEF(double east, double north, double up) const {
    const double sinLat = std::sin(origin_.latitude);
    const double cosLat = std::cos(origin_.latitude);
    const double sinLon = std::sin(origin_.longitude);
    const double cosLon = std::cos(origin_.longitude);

    ECEF e;
    e.x = -sinLon * east - sinLat * cosLon * north + cosLat * cosLon * up + originECEF_.x;
    e.y = cosLon * east - sinLat * sinLon * north + cosLat * sinLon * up + originECEF_.y;
    e.z = cosLat * north + sinLat * up + originECEF_.z;
    return e;
}

ECEF CoordinateConverter::enuToECEF(const ENU& enu) const {
    return enuToECEF(enu.east, enu.north, enu.up);
}

// -----------------------------------------------------------------------------
// Geodetic ↔ ENU (convenience)
// -----------------------------------------------------------------------------

ENU CoordinateConverter::geodeticToENU(double lat_rad, double lon_rad, double alt_m) const {
    return ecefToENU(geodeticToECEF(lat_rad, lon_rad, alt_m));
}

ENU CoordinateConverter::geodeticToENU(const Geodetic& g) const {
    return geodeticToENU(g.latitude, g.longitude, g.altitude);
}

Geodetic CoordinateConverter::enuToGeodetic(double east, double north, double up) const {
    return ecefToGeodetic(enuToECEF(east, north, up));
}

Geodetic CoordinateConverter::enuToGeodetic(const ENU& e) const {
    return enuToGeodetic(e.east, e.north, e.up);
}

// -----------------------------------------------------------------------------
// Helpers
// -----------------------------------------------------------------------------

double CoordinateConverter::degToRad(double deg) {
    return deg * (3.14159265358979323846 / 180.0);
}

double CoordinateConverter::radToDeg(double rad) {
    return rad * (180.0 / 3.14159265358979323846);
}

}  // namespace math
}  // namespace engine
