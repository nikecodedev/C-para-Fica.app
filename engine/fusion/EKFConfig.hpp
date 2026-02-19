#pragma once

#include "../math/Matrix.hpp"
#include <cmath>

namespace engine {
namespace fusion {

/** State dimension: 3 pos + 3 vel + 4 quat + 3 gyro bias + 3 accel bias = 16 */
constexpr int EKF_STATE_DIM = 16;

/** Indices into state vector. */
namespace StateIndex {
    constexpr int PX = 0, PY = 1, PZ = 2;
    constexpr int VX = 3, VY = 4, VZ = 5;
    constexpr int QW = 6, QX = 7, QY = 8, QZ = 9;
    constexpr int BGX = 10, BGY = 11, BGZ = 12;
    constexpr int BAX = 13, BAY = 14, BAZ = 15;
}

/**
 * Configurable noise covariance matrices for the EKF.
 * All matrices are diagonal (or can be extended to full).
 */
struct EKFConfig {
    /** Process noise covariance Q (state perturbation per dt).
     * Diagonal: [pos, vel, quat, gyro_bias, accel_bias] variances. */
    double process_pos{0.01};      ///< m²
    double process_vel{0.1};      ///< (m/s)²
    double process_quat{1e-6};   ///< quat component variance
    double process_gyro_bias{1e-8};  ///< (rad/s)²
    double process_accel_bias{1e-8}; ///< (m/s²)²

    /** GPS measurement noise R (m²). */
    double gps_pos{1.0};          ///< position variance per axis

    /** IMU measurement noise R (accel: (m/s²)², gyro: (rad/s)²). */
    double imu_accel{0.1};
    double imu_gyro{0.01};

    /** Magnetometer measurement noise R (µT²). */
    double magnetometer{0.1};

    /** Gravity magnitude (m/s²). */
    double gravity{9.80665};

    /** Build process noise Q matrix for given dt. */
    math::Matrix<EKF_STATE_DIM, EKF_STATE_DIM> buildProcessNoise(double dt) const;

    /** Build GPS measurement noise R (3x3). */
    math::Matrix<3, 3> buildGPSMeasurementNoise() const;

    /** Build IMU measurement noise R (6x6: accel + gyro). */
    math::Matrix<6, 6> buildIMUMeasurementNoise() const;

    /** Build magnetometer measurement noise R (3x3). */
    math::Matrix<3, 3> buildMagnetometerMeasurementNoise() const;
};

}  // namespace fusion
}  // namespace engine
