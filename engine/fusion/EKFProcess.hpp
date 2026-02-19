#pragma once

#include "EKFConfig.hpp"
#include "../math/Quaternion.hpp"

namespace engine {
namespace fusion {

/**
 * Process model for the EKF.
 * State dynamics: position, velocity, quaternion, biases.
 * Uses IMU (accel, gyro) as input.
 */
class EKFProcess {
public:
    using StateVec = math::Matrix<EKF_STATE_DIM, 1>;
    using StateCov = math::Matrix<EKF_STATE_DIM, EKF_STATE_DIM>;

    /** Apply process model: x_pred = f(x, accel, gyro, dt) */
    void predictState(const StateVec& x, double ax, double ay, double az,
                      double gx, double gy, double gz, double dt,
                      StateVec& x_pred) const;

    /** Compute state transition Jacobian F = df/dx at (x, accel, gyro, dt) */
    void computeJacobian(const StateVec& x, double ax, double ay, double az,
                         double gx, double gy, double gz, double dt,
                         StateCov& F) const;

    void setGravity(double g) { gravity_ = g; }

private:
    double gravity_{9.80665};

    /** Rotation matrix from quaternion in state. */
    void getRotation(const StateVec& x, double R[3][3]) const;
};

}  // namespace fusion
}  // namespace engine
