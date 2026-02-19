#include "EKFConfig.hpp"

namespace engine {
namespace fusion {

using math::Matrix;

Matrix<EKF_STATE_DIM, EKF_STATE_DIM> EKFConfig::buildProcessNoise(double dt) const {
    Matrix<EKF_STATE_DIM, EKF_STATE_DIM> Q;
    Q.setZero();
    const double dt2 = dt * dt;
    Q(0, 0) = process_pos * dt2;
    Q(1, 1) = process_pos * dt2;
    Q(2, 2) = process_pos * dt2;
    Q(3, 3) = process_vel * dt;
    Q(4, 4) = process_vel * dt;
    Q(5, 5) = process_vel * dt;
    Q(6, 6) = Q(7, 7) = Q(8, 8) = Q(9, 9) = process_quat * dt;
    Q(10, 10) = Q(11, 11) = Q(12, 12) = process_gyro_bias * dt;
    Q(13, 13) = Q(14, 14) = Q(15, 15) = process_accel_bias * dt;
    return Q;
}

Matrix<3, 3> EKFConfig::buildGPSMeasurementNoise() const {
    Matrix<3, 3> R;
    R.setZero();
    R(0, 0) = R(1, 1) = R(2, 2) = gps_pos;
    return R;
}

Matrix<6, 6> EKFConfig::buildIMUMeasurementNoise() const {
    Matrix<6, 6> R;
    R.setZero();
    R(0, 0) = R(1, 1) = R(2, 2) = imu_accel;
    R(3, 3) = R(4, 4) = R(5, 5) = imu_gyro;
    return R;
}

Matrix<3, 3> EKFConfig::buildMagnetometerMeasurementNoise() const {
    Matrix<3, 3> R;
    R.setZero();
    R(0, 0) = R(1, 1) = R(2, 2) = magnetometer;
    return R;
}

}  // namespace fusion
}  // namespace engine
