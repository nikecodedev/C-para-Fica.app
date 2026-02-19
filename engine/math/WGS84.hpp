#pragma once

namespace engine {
namespace math {

/** WGS84 ellipsoid constants. */
namespace WGS84 {
    constexpr double A = 6378137.0;              ///< Semi-major axis (m)
    constexpr double F = 1.0 / 298.257223563;    ///< Flattening
    constexpr double E2 = 2.0 * F - F * F;       ///< Eccentricity squared
}

}  // namespace math
}  // namespace engine
