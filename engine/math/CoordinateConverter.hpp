#pragma once

#include "WGS84.hpp"
#include <cmath>
#include <array>

namespace engine {
namespace math {

/** Earth-Centered Earth-Fixed coordinates (meters). */
struct ECEF {
    double x{0.0};
    double y{0.0};
    double z{0.0};
};

/** East-North-Up local tangent plane (meters). */
struct ENU {
    double east{0.0};
    double north{0.0};
    double up{0.0};
};

/** Geodetic coordinates (WGS84). */
struct Geodetic {
    double latitude{0.0};   ///< Radians
    double longitude{0.0};  ///< Radians
    double altitude{0.0};   ///< Meters
};

/**
 * Converts between WGS84 geodetic, ECEF, and ENU local tangent plane.
 * Maintains consistent world frame for the tracking engine.
 */
class CoordinateConverter {
public:
    /**
     * Set the local tangent plane origin (ENU reference point).
     * Must be called before ECEF↔ENU conversions.
     */
    void setOrigin(double lat_rad, double lon_rad, double alt_m);
    void setOrigin(const Geodetic& origin);

    Geodetic getOrigin() const { return origin_; }

    // ----- Lat/Lon (radians) ↔ ECEF -----

    /** Geodetic (rad, rad, m) → ECEF (m). */
    ECEF geodeticToECEF(double lat_rad, double lon_rad, double alt_m) const;
    ECEF geodeticToECEF(const Geodetic& g) const;

    /** ECEF (m) → Geodetic (rad, rad, m). */
    Geodetic ecefToGeodetic(double x, double y, double z) const;
    Geodetic ecefToGeodetic(const ECEF& e) const;

    // ----- ECEF ↔ ENU -----

    /** ECEF (m) → ENU (m) relative to origin. */
    ENU ecefToENU(double x, double y, double z) const;
    ENU ecefToENU(const ECEF& e) const;

    /** ENU (m) → ECEF (m). */
    ECEF enuToECEF(double east, double north, double up) const;
    ECEF enuToECEF(const ENU& e) const;

    // ----- Lat/Lon (radians) ↔ ENU -----

    /** Geodetic → ENU (convenience: geodetic→ECEF→ENU). */
    ENU geodeticToENU(double lat_rad, double lon_rad, double alt_m) const;
    ENU geodeticToENU(const Geodetic& g) const;

    /** ENU → Geodetic (convenience: ENU→ECEF→geodetic). */
    Geodetic enuToGeodetic(double east, double north, double up) const;
    Geodetic enuToGeodetic(const ENU& e) const;

    /** Degrees → radians / radians → degrees helpers. */
    static double degToRad(double deg);
    static double radToDeg(double rad);

private:
    Geodetic origin_{0.0, 0.0, 0.0};
    ECEF originECEF_{0.0, 0.0, 0.0};
    bool originSet_{false};

    void updateOriginECEF();
};

}  // namespace math
}  // namespace engine
