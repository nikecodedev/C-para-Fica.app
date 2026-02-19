#include "EKFProcess.hpp"

namespace engine {
namespace fusion {

using namespace StateIndex;

void EKFProcess::getRotation(const StateVec& x, double R[3][3]) const {
    math::Quaternion q{x(6), x(7), x(8), x(9)};
    q.normalize();
    q.toRotationMatrix(R);
}

void EKFProcess::predictState(const StateVec& x, double ax, double ay, double az,
                               double gx, double gy, double gz, double dt,
                               StateVec& x_pred) const {
    x_pred = x;

    math::Quaternion q{x(6), x(7), x(8), x(9)};
    q.normalize();

    double R[3][3];
    q.toRotationMatrix(R);

    const double bax = x(BAX), bay = x(BAY), baz = x(BAZ);
    const double bgx = x(BGX), bgy = x(BGY), bgz = x(BGZ);

    double accel_body[3] = {ax - bax, ay - bay, az - baz};
    double accel_world[3];
    for (int i = 0; i < 3; ++i) {
        accel_world[i] = R[0][i]*accel_body[0] + R[1][i]*accel_body[1] + R[2][i]*accel_body[2];
    }
    accel_world[2] -= gravity_;

    x_pred(PX) = x(PX) + x(VX)*dt + 0.5*accel_world[0]*dt*dt;
    x_pred(PY) = x(PY) + x(VY)*dt + 0.5*accel_world[1]*dt*dt;
    x_pred(PZ) = x(PZ) + x(VZ)*dt + 0.5*accel_world[2]*dt*dt;

    x_pred(VX) = x(VX) + accel_world[0]*dt;
    x_pred(VY) = x(VY) + accel_world[1]*dt;
    x_pred(VZ) = x(VZ) + accel_world[2]*dt;

    const double wx = (gx - bgx) * dt;
    const double wy = (gy - bgy) * dt;
    const double wz = (gz - bgz) * dt;
    math::Quaternion dq = math::Quaternion::fromDelta(wx, wy, wz);
    math::Quaternion q_new = q * dq;
    q_new.normalize();

    x_pred(QW) = q_new.w;
    x_pred(QX) = q_new.x;
    x_pred(QY) = q_new.y;
    x_pred(QZ) = q_new.z;

    x_pred(BGX) = x(BGX);
    x_pred(BGY) = x(BGY);
    x_pred(BGZ) = x(BGZ);
    x_pred(BAX) = x(BAX);
    x_pred(BAY) = x(BAY);
    x_pred(BAZ) = x(BAZ);
}

void EKFProcess::computeJacobian(const StateVec& x, double ax, double ay, double az,
                                  double gx, double gy, double gz, double dt,
                                  StateCov& F) const {
    F.setZero();

    double R[3][3];
    getRotation(x, R);

    const double bax = x(BAX), bay = x(BAY), baz = x(BAZ);
    const double bgx = x(BGX), bgy = x(BGY), bgz = x(BGZ);

    double accel_body[3] = {ax - bax, ay - bay, az - baz};
    double accel_world[3];
    for (int i = 0; i < 3; ++i) {
        accel_world[i] = R[0][i]*accel_body[0] + R[1][i]*accel_body[1] + R[2][i]*accel_body[2];
    }
    accel_world[2] -= gravity_;

    const double dt2 = dt * dt * 0.5;

    F(PX, PX) = F(PY, PY) = F(PZ, PZ) = 1.0;
    F(PX, VX) = F(PY, VY) = F(PZ, VZ) = dt;
    F(PX, PX) = 1; F(PX, VX) = dt; F(PX, QW) = 0; F(PX, QX) = dt2 * (-2*R[0][1]*accel_body[2] + 2*R[0][2]*accel_body[1]);
    F(PX, QY) = dt2 * ( 2*R[0][0]*accel_body[2] - 2*R[0][2]*accel_body[0]);
    F(PX, QZ) = dt2 * (-2*R[0][0]*accel_body[1] + 2*R[0][1]*accel_body[0]);
    F(PX, BAX) = -dt2 * R[0][0]; F(PX, BAY) = -dt2 * R[0][1]; F(PX, BAZ) = -dt2 * R[0][2];

    F(PY, PY) = 1; F(PY, VY) = dt;
    F(PY, QW) = 0; F(PY, QX) = dt2 * (-2*R[1][1]*accel_body[2] + 2*R[1][2]*accel_body[1]);
    F(PY, QY) = dt2 * ( 2*R[1][0]*accel_body[2] - 2*R[1][2]*accel_body[0]);
    F(PY, QZ) = dt2 * (-2*R[1][0]*accel_body[1] + 2*R[1][1]*accel_body[0]);
    F(PY, BAX) = -dt2 * R[1][0]; F(PY, BAY) = -dt2 * R[1][1]; F(PY, BAZ) = -dt2 * R[1][2];

    F(PZ, PZ) = 1; F(PZ, VZ) = dt;
    F(PZ, QW) = 0; F(PZ, QX) = dt2 * (-2*R[2][1]*accel_body[2] + 2*R[2][2]*accel_body[1]);
    F(PZ, QY) = dt2 * ( 2*R[2][0]*accel_body[2] - 2*R[2][2]*accel_body[0]);
    F(PZ, QZ) = dt2 * (-2*R[2][0]*accel_body[1] + 2*R[2][1]*accel_body[0]);
    F(PZ, BAX) = -dt2 * R[2][0]; F(PZ, BAY) = -dt2 * R[2][1]; F(PZ, BAZ) = -dt2 * R[2][2];

    F(VX, VX) = F(VY, VY) = F(VZ, VZ) = 1.0;
    F(VX, QX) = dt * (-2*R[0][1]*accel_body[2] + 2*R[0][2]*accel_body[1]);
    F(VX, QY) = dt * ( 2*R[0][0]*accel_body[2] - 2*R[0][2]*accel_body[0]);
    F(VX, QZ) = dt * (-2*R[0][0]*accel_body[1] + 2*R[0][1]*accel_body[0]);
    F(VX, BAX) = -dt * R[0][0]; F(VX, BAY) = -dt * R[0][1]; F(VX, BAZ) = -dt * R[0][2];

    F(VY, QX) = dt * (-2*R[1][1]*accel_body[2] + 2*R[1][2]*accel_body[1]);
    F(VY, QY) = dt * ( 2*R[1][0]*accel_body[2] - 2*R[1][2]*accel_body[0]);
    F(VY, QZ) = dt * (-2*R[1][0]*accel_body[1] + 2*R[1][1]*accel_body[0]);
    F(VY, BAX) = -dt * R[1][0]; F(VY, BAY) = -dt * R[1][1]; F(VY, BAZ) = -dt * R[1][2];

    F(VZ, QX) = dt * (-2*R[2][1]*accel_body[2] + 2*R[2][2]*accel_body[1]);
    F(VZ, QY) = dt * ( 2*R[2][0]*accel_body[2] - 2*R[2][2]*accel_body[0]);
    F(VZ, QZ) = dt * (-2*R[2][0]*accel_body[1] + 2*R[2][1]*accel_body[0]);
    F(VZ, BAX) = -dt * R[2][0]; F(VZ, BAY) = -dt * R[2][1]; F(VZ, BAZ) = -dt * R[2][2];

    const double qw = x(QW), qx = x(QX), qy = x(QY), qz = x(QZ);
    const double wx = (gx - bgx) * 0.5 * dt, wy = (gy - bgy) * 0.5 * dt, wz = (gz - bgz) * 0.5 * dt;
    F(QW, QW) = 1; F(QW, QX) = -wx; F(QW, QY) = -wy; F(QW, QZ) = -wz;
    F(QW, BGX) =  qx*dt*0.5; F(QW, BGY) =  qy*dt*0.5; F(QW, BGZ) =  qz*dt*0.5;
    F(QX, QW) =  wx; F(QX, QX) = 1; F(QX, QY) =  wz; F(QX, QZ) = -wy;
    F(QX, BGX) = -qw*dt*0.5; F(QX, BGY) =  qz*dt*0.5; F(QX, BGZ) = -qy*dt*0.5;
    F(QY, QW) =  wy; F(QY, QX) = -wz; F(QY, QY) = 1; F(QY, QZ) =  wx;
    F(QY, BGX) = -qz*dt*0.5; F(QY, BGY) = -qw*dt*0.5; F(QY, BGZ) =  qx*dt*0.5;
    F(QZ, QW) =  wz; F(QZ, QX) =  wy; F(QZ, QY) = -wx; F(QZ, QZ) = 1;
    F(QZ, BGX) =  qy*dt*0.5; F(QZ, BGY) = -qx*dt*0.5; F(QZ, BGZ) = -qw*dt*0.5;

    F(BGX, BGX) = F(BGY, BGY) = F(BGZ, BGZ) = 1.0;
    F(BAX, BAX) = F(BAY, BAY) = F(BAZ, BAZ) = 1.0;
}

}  // namespace fusion
}  // namespace engine
