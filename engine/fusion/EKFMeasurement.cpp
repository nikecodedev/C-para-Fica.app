#include "EKFMeasurement.hpp"
#include "../math/Quaternion.hpp"
#include <cmath>

namespace engine {
namespace fusion {

using namespace StateIndex;

void EKFMeasurement::gpsPrediction(const StateVec& x, double h[3]) {
    h[0] = x(PX);
    h[1] = x(PY);
    h[2] = x(PZ);
}

void EKFMeasurement::gpsJacobian(math::Matrix<3, EKF_STATE_DIM>& H) {
    H.setZero();
    H(0, PX) = 1;
    H(1, PY) = 1;
    H(2, PZ) = 1;
}

void EKFMeasurement::imuPrediction(const StateVec& x, double gravity, double h[6]) {
    math::Quaternion q{x(QW), x(QX), x(QY), x(QZ)};
    q.normalize();
    math::Quaternion qInv{q.w, -q.x, -q.y, -q.z};
    double gx, gy, gz;
    rotateVector(qInv, 0, 0, -gravity, gx, gy, gz);
    h[0] = gx;
    h[1] = gy;
    h[2] = gz;
    h[3] = x(BGX);
    h[4] = x(BGY);
    h[5] = x(BGZ);
}

void EKFMeasurement::imuJacobian(const StateVec& x, double gravity, math::Matrix<6, EKF_STATE_DIM>& H) {
    H.setZero();
    double h0[6];
    imuPrediction(x, gravity, h0);
    for (int j = 0; j < EKF_STATE_DIM; ++j) {
        StateVec xp = x;
        xp(j, 0) += JACOBIAN_EPS;
        if (j >= 6 && j <= 9) {
            math::Quaternion q{xp(6), xp(7), xp(8), xp(9)};
            q.normalize();
            xp(6) = q.w; xp(7) = q.x; xp(8) = q.y; xp(9) = q.z;
        }
        double hp[6];
        imuPrediction(xp, gravity, hp);
        for (int i = 0; i < 6; ++i) H(i, j) = (hp[i] - h0[i]) / JACOBIAN_EPS;
    }
}

void EKFMeasurement::magnetometerPrediction(const StateVec& x, double mx_earth, double my_earth, double mz_earth, double h[3]) {
    math::Quaternion q{x(QW), x(QX), x(QY), x(QZ)};
    q.normalize();
    math::Quaternion qInv{q.w, -q.x, -q.y, -q.z};  // conjugate for world-to-body
    double mx_b, my_b, mz_b;
    rotateVector(qInv, mx_earth, my_earth, mz_earth, mx_b, my_b, mz_b);
    h[0] = mx_b;
    h[1] = my_b;
    h[2] = mz_b;
}

void EKFMeasurement::magnetometerJacobian(const StateVec& x, double mx_earth, double my_earth, double mz_earth,
                                          math::Matrix<3, EKF_STATE_DIM>& H) {
    H.setZero();
    double h0[3];
    magnetometerPrediction(x, mx_earth, my_earth, mz_earth, h0);
    for (int j = 6; j < 10; ++j) {
        StateVec xp = x;
        xp(j, 0) += JACOBIAN_EPS;
        math::Quaternion q{xp(6), xp(7), xp(8), xp(9)};
        q.normalize();
        xp(6) = q.w; xp(7) = q.x; xp(8) = q.y; xp(9) = q.z;
        double hp[3];
        magnetometerPrediction(xp, mx_earth, my_earth, mz_earth, hp);
        H(0, j) = (hp[0] - h0[0]) / JACOBIAN_EPS;
        H(1, j) = (hp[1] - h0[1]) / JACOBIAN_EPS;
        H(2, j) = (hp[2] - h0[2]) / JACOBIAN_EPS;
    }
}

}  // namespace fusion
}  // namespace engine
