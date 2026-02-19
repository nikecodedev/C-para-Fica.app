#pragma once

#include <array>
#include <cmath>

namespace engine {
namespace math {

/** Quaternion (qw, qx, qy, qz) for orientation. */
struct Quaternion {
    double w{1.0};
    double x{0.0};
    double y{0.0};
    double z{0.0};

    /** Identity quaternion. */
    static Quaternion identity() { return {1.0, 0.0, 0.0, 0.0}; }

    /** Product: this * other. */
    Quaternion operator*(const Quaternion& other) const;

    /** Normalize in place, return *this. */
    Quaternion& normalize();

    /** Return normalized copy. */
    Quaternion normalized() const;

    /** Rotation matrix from quaternion (3x3, body-to-world). */
    void toRotationMatrix(double R[3][3]) const;

    /** Build quaternion from rotation matrix (3x3). */
    static Quaternion fromRotationMatrix(const double R[3][3]);

    /** Build quaternion from axis-angle (axis normalized, angle in rad). */
    static Quaternion fromAxisAngle(double ax, double ay, double az, double angle_rad);

    /** Build quaternion from small rotation vector [dx, dy, dz] (rad). */
    static Quaternion fromDelta(double dx, double dy, double dz);
};

/** Rotate vector v by quaternion q (v_world = q * v_body * q^-1). */
void rotateVector(const Quaternion& q, double vx, double vy, double vz,
                  double& out_x, double& out_y, double& out_z);

}  // namespace math
}  // namespace engine
