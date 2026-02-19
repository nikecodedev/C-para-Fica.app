#include "Quaternion.hpp"
#include <cmath>
#include <algorithm>

namespace engine {
namespace math {

Quaternion Quaternion::operator*(const Quaternion& other) const {
    Quaternion r;
    r.w = w*other.w - x*other.x - y*other.y - z*other.z;
    r.x = w*other.x + x*other.w + y*other.z - z*other.y;
    r.y = w*other.y - x*other.z + y*other.w + z*other.x;
    r.z = w*other.z + x*other.y - y*other.x + z*other.w;
    return r;
}

Quaternion& Quaternion::normalize() {
    const double n = std::sqrt(w*w + x*x + y*y + z*z);
    if (n > 1e-12) {
        w /= n; x /= n; y /= n; z /= n;
    }
    return *this;
}

Quaternion Quaternion::normalized() const {
    Quaternion q = *this;
    return q.normalize();
}

void Quaternion::toRotationMatrix(double R[3][3]) const {
    const double qw = w, qx = x, qy = y, qz = z;
    R[0][0] = 1 - 2*(qy*qy + qz*qz);
    R[0][1] = 2*(qx*qy - qz*qw);
    R[0][2] = 2*(qx*qz + qy*qw);
    R[1][0] = 2*(qx*qy + qz*qw);
    R[1][1] = 1 - 2*(qx*qx + qz*qz);
    R[1][2] = 2*(qy*qz - qx*qw);
    R[2][0] = 2*(qx*qz - qy*qw);
    R[2][1] = 2*(qy*qz + qx*qw);
    R[2][2] = 1 - 2*(qx*qx + qy*qy);
}

Quaternion Quaternion::fromRotationMatrix(const double R[3][3]) {
    const double t = R[0][0] + R[1][1] + R[2][2];
    Quaternion q;
    if (t > 0) {
        const double s = 0.5 / std::sqrt(t + 1.0);
        q.w = 0.25 / s;
        q.x = (R[2][1] - R[1][2]) * s;
        q.y = (R[0][2] - R[2][0]) * s;
        q.z = (R[1][0] - R[0][1]) * s;
    } else if (R[0][0] > R[1][1] && R[0][0] > R[2][2]) {
        const double s = 2.0 * std::sqrt(1.0 + R[0][0] - R[1][1] - R[2][2]);
        q.w = (R[2][1] - R[1][2]) / s;
        q.x = 0.25 * s;
        q.y = (R[0][1] + R[1][0]) / s;
        q.z = (R[0][2] + R[2][0]) / s;
    } else if (R[1][1] > R[2][2]) {
        const double s = 2.0 * std::sqrt(1.0 + R[1][1] - R[0][0] - R[2][2]);
        q.w = (R[0][2] - R[2][0]) / s;
        q.x = (R[0][1] + R[1][0]) / s;
        q.y = 0.25 * s;
        q.z = (R[1][2] + R[2][1]) / s;
    } else {
        const double s = 2.0 * std::sqrt(1.0 + R[2][2] - R[0][0] - R[1][1]);
        q.w = (R[1][0] - R[0][1]) / s;
        q.x = (R[0][2] + R[2][0]) / s;
        q.y = (R[1][2] + R[2][1]) / s;
        q.z = 0.25 * s;
    }
    return q.normalized();
}

Quaternion Quaternion::fromAxisAngle(double ax, double ay, double az, double angle_rad) {
    const double ha = 0.5 * angle_rad;
    const double s = std::sin(ha);
    const double n = std::sqrt(ax*ax + ay*ay + az*az);
    const double f = (n > 1e-12) ? (s / n) : 0.0;
    return Quaternion{std::cos(ha), ax*f, ay*f, az*f}.normalized();
}

Quaternion Quaternion::fromDelta(double dx, double dy, double dz) {
    const double angle = std::sqrt(dx*dx + dy*dy + dz*dz);
    if (angle < 1e-12) return identity();
    return fromAxisAngle(dx/angle, dy/angle, dz/angle, angle);
}

void rotateVector(const Quaternion& q, double vx, double vy, double vz,
                  double& out_x, double& out_y, double& out_z) {
    const double qw = q.w, qx = q.x, qy = q.y, qz = q.z;
    const double tx = 2.0 * (qy*vz - qz*vy);
    const double ty = 2.0 * (qz*vx - qx*vz);
    const double tz = 2.0 * (qx*vy - qy*vx);
    out_x = vx + qw*tx + (qy*tz - qz*ty);
    out_y = vy + qw*ty + (qz*tx - qx*tz);
    out_z = vz + qw*tz + (qx*ty - qy*tx);
}

}  // namespace math
}  // namespace engine
