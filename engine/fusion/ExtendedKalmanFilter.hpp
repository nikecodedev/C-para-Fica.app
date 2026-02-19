#pragma once

#include "EKFConfig.hpp"
#include "EKFProcess.hpp"
#include "EKFMeasurement.hpp"
#include "../math/Matrix.hpp"
#include "../math/Quaternion.hpp"

namespace engine {
namespace fusion {

/**
 * Extended Kalman Filter for GPS/IMU/Magnetometer fusion.
 * State: position, velocity, quaternion, gyro bias, accel bias (16D).
 */
class ExtendedKalmanFilter {
public:
    using StateVec = math::Matrix<EKF_STATE_DIM, 1>;
    using StateCov = math::Matrix<EKF_STATE_DIM, EKF_STATE_DIM>;

    ExtendedKalmanFilter();

    /** Set config (noise covariances, gravity). */
    void setConfig(const EKFConfig& config);
    const EKFConfig& getConfig() const { return config_; }

    /** Set initial state. */
    void setState(const StateVec& x);
    void setState(double px, double py, double pz,
                  double vx, double vy, double vz,
                  const math::Quaternion& q,
                  double bgx, double bgy, double bgz,
                  double bax, double bay, double baz);

    /** Set initial covariance. */
    void setCovariance(const StateCov& P);

    /** Get current state and covariance. */
    const StateVec& getState() const { return x_; }
    const StateCov& getCovariance() const { return P_; }

    /** Set last IMU reading (used by predict). */
    void setImuInput(double ax, double ay, double az, double gx, double gy, double gz);

    /** Predict step: x = f(x, imu, dt), P = F*P*F' + Q. */
    void predict(double dt);

    /** GPS update: z = [px, py, pz] in ENU. */
    void updateGPS(double px, double py, double pz);

    /** IMU update: z = [ax, ay, az, gx, gy, gz]. Helps bias and orientation. */
    void updateIMU(double ax, double ay, double az, double gx, double gy, double gz);

    /** Magnetometer update: z = [mx, my, mz] in body frame (µT). */
    void updateMagnetometer(double mx, double my, double mz);

    /** Earth magnetic field in ENU for mag update (default North). */
    void setEarthMagneticField(double mx, double my, double mz);

private:
    EKFConfig config_;
    EKFProcess process_;
    StateVec x_;
    StateCov P_;

    double lastAccel_[3]{0, 0, 0};
    double lastGyro_[3]{0, 0, 0};
    bool hasImu_{false};

    double earthMag_[3]{0.0, 1.0, 0.0};  // ENU North default
};

}  // namespace fusion
}  // namespace engine
