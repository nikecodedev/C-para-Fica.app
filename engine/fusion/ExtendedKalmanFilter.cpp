#include "ExtendedKalmanFilter.hpp"

namespace engine {
namespace fusion {

using math::Matrix;

ExtendedKalmanFilter::ExtendedKalmanFilter() {
    x_.setZero();
    x_(StateIndex::QW, 0) = 1.0;
    P_.setIdentity();
    for (int i = 0; i < EKF_STATE_DIM; ++i) {
        P_(i, i) *= 10.0;
    }
}

void ExtendedKalmanFilter::setConfig(const EKFConfig& config) {
    config_ = config;
    process_.setGravity(config.gravity);
}

void ExtendedKalmanFilter::setState(const StateVec& x) {
    x_ = x;
}

void ExtendedKalmanFilter::setState(double px, double py, double pz,
                                    double vx, double vy, double vz,
                                    const math::Quaternion& q,
                                    double bgx, double bgy, double bgz,
                                    double bax, double bay, double baz) {
    x_.setZero();
    x_(StateIndex::PX, 0) = px;
    x_(StateIndex::PY, 0) = py;
    x_(StateIndex::PZ, 0) = pz;
    x_(StateIndex::VX, 0) = vx;
    x_(StateIndex::VY, 0) = vy;
    x_(StateIndex::VZ, 0) = vz;
    math::Quaternion qn = q.normalized();
    x_(StateIndex::QW, 0) = qn.w;
    x_(StateIndex::QX, 0) = qn.x;
    x_(StateIndex::QY, 0) = qn.y;
    x_(StateIndex::QZ, 0) = qn.z;
    x_(StateIndex::BGX, 0) = bgx;
    x_(StateIndex::BGY, 0) = bgy;
    x_(StateIndex::BGZ, 0) = bgz;
    x_(StateIndex::BAX, 0) = bax;
    x_(StateIndex::BAY, 0) = bay;
    x_(StateIndex::BAZ, 0) = baz;
}

void ExtendedKalmanFilter::setCovariance(const StateCov& P) {
    P_ = P;
}

void ExtendedKalmanFilter::setImuInput(double ax, double ay, double az, double gx, double gy, double gz) {
    lastAccel_[0] = ax;
    lastAccel_[1] = ay;
    lastAccel_[2] = az;
    lastGyro_[0] = gx;
    lastGyro_[1] = gy;
    lastGyro_[2] = gz;
    hasImu_ = true;
}

void ExtendedKalmanFilter::predict(double dt) {
    double ax = lastAccel_[0], ay = lastAccel_[1], az = lastAccel_[2];
    double gx = lastGyro_[0], gy = lastGyro_[1], gz = lastGyro_[2];
    if (!hasImu_) {
        ax = ay = az = 0;
        gx = gy = gz = 0;
    }

    StateVec x_pred;
    process_.predictState(x_, ax, ay, az, gx, gy, gz, dt, x_pred);

    StateCov F;
    process_.computeJacobian(x_, ax, ay, az, gx, gy, gz, dt, F);

    StateCov Q = config_.buildProcessNoise(dt);
    StateCov P_pred = F * P_ * F.transposed() + Q;

    x_ = x_pred;
    P_ = P_pred;

    math::Quaternion q{x_(StateIndex::QW, 0), x_(StateIndex::QX, 0), x_(StateIndex::QY, 0), x_(StateIndex::QZ, 0)};
    q.normalize();
    x_(StateIndex::QW, 0) = q.w;
    x_(StateIndex::QX, 0) = q.x;
    x_(StateIndex::QY, 0) = q.y;
    x_(StateIndex::QZ, 0) = q.z;
}

void ExtendedKalmanFilter::updateGPS(double px, double py, double pz) {
    double h[3];
    EKFMeasurement::gpsPrediction(x_, h);

    Matrix<3, EKF_STATE_DIM> H;
    EKFMeasurement::gpsJacobian(H);

    Matrix<3, 3> R = config_.buildGPSMeasurementNoise();

    Matrix<3, 1> z;
    z(0, 0) = px;
    z(1, 0) = py;
    z(2, 0) = pz;

    Matrix<3, 1> h_vec;
    h_vec(0, 0) = h[0];
    h_vec(1, 0) = h[1];
    h_vec(2, 0) = h[2];

    Matrix<3, EKF_STATE_DIM> Ht = H.transposed();
    Matrix<3, 3> S = H * P_ * Ht + R;

    Matrix<3, 3> S_inv;
    if (!math::invert(S, S_inv)) return;

    Matrix<EKF_STATE_DIM, 3> K = P_ * Ht * S_inv;
    Matrix<3, 1> dz;
    dz(0, 0) = z(0, 0) - h_vec(0, 0);
    dz(1, 0) = z(1, 0) - h_vec(1, 0);
    dz(2, 0) = z(2, 0) - h_vec(2, 0);

    Matrix<EKF_STATE_DIM, 1> dx = K * dz;
    for (int i = 0; i < EKF_STATE_DIM; ++i) x_(i, 0) += dx(i, 0);

    StateCov I;
    I.setIdentity();
    P_ = (I - K * H) * P_;

    math::Quaternion q{x_(StateIndex::QW, 0), x_(StateIndex::QX, 0), x_(StateIndex::QY, 0), x_(StateIndex::QZ, 0)};
    q.normalize();
    x_(StateIndex::QW, 0) = q.w;
    x_(StateIndex::QX, 0) = q.x;
    x_(StateIndex::QY, 0) = q.y;
    x_(StateIndex::QZ, 0) = q.z;
}

void ExtendedKalmanFilter::updateIMU(double ax, double ay, double az, double gx, double gy, double gz) {
    double h[6];
    EKFMeasurement::imuPrediction(x_, config_.gravity, h);

    Matrix<6, EKF_STATE_DIM> H;
    EKFMeasurement::imuJacobian(x_, config_.gravity, H);

    Matrix<6, 6> R = config_.buildIMUMeasurementNoise();

    Matrix<6, 1> z;
    z(0, 0) = ax; z(1, 0) = ay; z(2, 0) = az;
    z(3, 0) = gx; z(4, 0) = gy; z(5, 0) = gz;

    Matrix<6, 1> h_vec;
    for (int i = 0; i < 6; ++i) h_vec(i, 0) = h[i];

    Matrix<6, EKF_STATE_DIM> Ht = H.transposed();
    Matrix<6, 6> S = H * P_ * Ht + R;

    Matrix<6, 6> S_inv;
    if (!math::invert(S, S_inv)) return;

    Matrix<EKF_STATE_DIM, 6> K = P_ * Ht * S_inv;
    Matrix<6, 1> dz;
    for (int i = 0; i < 6; ++i) dz(i, 0) = z(i, 0) - h_vec(i, 0);

    Matrix<EKF_STATE_DIM, 1> dx = K * dz;
    for (int i = 0; i < EKF_STATE_DIM; ++i) x_(i, 0) += dx(i, 0);

    StateCov I;
    I.setIdentity();
    P_ = (I - K * H) * P_;

    math::Quaternion q{x_(StateIndex::QW, 0), x_(StateIndex::QX, 0), x_(StateIndex::QY, 0), x_(StateIndex::QZ, 0)};
    q.normalize();
    x_(StateIndex::QW, 0) = q.w;
    x_(StateIndex::QX, 0) = q.x;
    x_(StateIndex::QY, 0) = q.y;
    x_(StateIndex::QZ, 0) = q.z;
}

void ExtendedKalmanFilter::updateMagnetometer(double mx, double my, double mz) {
    double h[3];
    EKFMeasurement::magnetometerPrediction(x_, earthMag_[0], earthMag_[1], earthMag_[2], h);

    Matrix<3, EKF_STATE_DIM> H;
    EKFMeasurement::magnetometerJacobian(x_, earthMag_[0], earthMag_[1], earthMag_[2], H);

    Matrix<3, 3> R = config_.buildMagnetometerMeasurementNoise();

    Matrix<3, 1> z;
    z(0, 0) = mx;
    z(1, 0) = my;
    z(2, 0) = mz;

    Matrix<3, 1> h_vec;
    h_vec(0, 0) = h[0];
    h_vec(1, 0) = h[1];
    h_vec(2, 0) = h[2];

    Matrix<3, EKF_STATE_DIM> Ht = H.transposed();
    Matrix<3, 3> S = H * P_ * Ht + R;

    Matrix<3, 3> S_inv;
    if (!math::invert(S, S_inv)) return;

    Matrix<EKF_STATE_DIM, 3> K = P_ * Ht * S_inv;
    Matrix<3, 1> dz;
    dz(0, 0) = z(0, 0) - h_vec(0, 0);
    dz(1, 0) = z(1, 0) - h_vec(1, 0);
    dz(2, 0) = z(2, 0) - h_vec(2, 0);

    Matrix<EKF_STATE_DIM, 1> dx = K * dz;
    for (int i = 0; i < EKF_STATE_DIM; ++i) x_(i, 0) += dx(i, 0);

    StateCov I;
    I.setIdentity();
    P_ = (I - K * H) * P_;

    math::Quaternion q{x_(StateIndex::QW, 0), x_(StateIndex::QX, 0), x_(StateIndex::QY, 0), x_(StateIndex::QZ, 0)};
    q.normalize();
    x_(StateIndex::QW, 0) = q.w;
    x_(StateIndex::QX, 0) = q.x;
    x_(StateIndex::QY, 0) = q.y;
    x_(StateIndex::QZ, 0) = q.z;
}

void ExtendedKalmanFilter::setEarthMagneticField(double mx, double my, double mz) {
    earthMag_[0] = mx;
    earthMag_[1] = my;
    earthMag_[2] = mz;
}