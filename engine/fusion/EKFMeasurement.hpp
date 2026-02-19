#pragma once

#include "EKFConfig.hpp"

namespace engine {
namespace fusion {

/**
 * Measurement models and Jacobians for the EKF.
 */
class EKFMeasurement {
public:
    using StateVec = math::Matrix<EKF_STATE_DIM, 1>;

    /** GPS: h(x) = [px, py, pz]. H is 3 x 16. */
    static void gpsPrediction(const StateVec& x, double h[3]);
    static void gpsJacobian(math::Matrix<3, EKF_STATE_DIM>& H);

    /** IMU: h(x) = [R^T * g; omega + bg]. Expected accel (gravity in body), gyro.
     * H is 6 x 16. Uses accel when stationary (gravity direction) and gyro. */
    static void imuPrediction(const StateVec& x, double gravity, double h[6]);
    static void imuJacobian(const StateVec& x, double gravity, math::Matrix<6, EKF_STATE_DIM>& H);

    /** Magnetometer: h(x) = R^T * m_earth. Expected mag in body frame.
     * m_earth is [0, 1, 0] ENU (North) or configurable. H is 3 x 16. */
    static void magnetometerPrediction(const StateVec& x, double mx_earth, double my_earth, double mz_earth, double h[3]);
    static void magnetometerJacobian(const StateVec& x, double mx_earth, double my_earth, double mz_earth,
                                      math::Matrix<3, EKF_STATE_DIM>& H);

    static constexpr double JACOBIAN_EPS = 1e-7;
};

}  // namespace fusion
}  // namespace engine
