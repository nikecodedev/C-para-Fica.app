/**
 * Unit tests for EKF math: Quaternion, CoordinateConverter, Matrix.
 * Never test fusion blindly in field—validate math first.
 */
#include "../math/Quaternion.hpp"
#include "../math/CoordinateConverter.hpp"
#include "../math/Matrix.hpp"
#include "../math/WGS84.hpp"
#include <cmath>
#include <iostream>

static int g_fail = 0;

#define ASSERT(cond, msg) do { if (!(cond)) { std::cerr << "FAIL: " << msg << std::endl; ++g_fail; } } while(0)
#define ASSERT_NEAR(a, b, eps, msg) ASSERT(std::abs((a)-(b)) < (eps), msg)

static void testQuaternion() {
    using namespace engine::math;

    Quaternion q = Quaternion::identity();
    ASSERT_NEAR(q.w, 1.0, 1e-9, "identity w");
    ASSERT_NEAR(q.x, 0.0, 1e-9, "identity x");

    Quaternion q2 = q * q;
    ASSERT_NEAR(q2.w, 1.0, 1e-9, "identity*identity");

    Quaternion q90 = Quaternion::fromAxisAngle(0, 0, 1, M_PI / 2);
    double ox, oy, oz;
    rotateVector(q90, 1, 0, 0, ox, oy, oz);
    ASSERT_NEAR(ox, 0.0, 1e-6, "rot90 x");
    ASSERT_NEAR(oy, 1.0, 1e-6, "rot90 y");

    double R[3][3];
    q90.toRotationMatrix(R);
    Quaternion qback = Quaternion::fromRotationMatrix(R);
    ASSERT_NEAR(q90.w, qback.w, 1e-6, "toMatrix fromMatrix w");
}

static void testCoordinateConverter() {
    using namespace engine::math;

    CoordinateConverter conv;
    conv.setOrigin(0, 0, 0);

    ECEF e = conv.geodeticToECEF(0, 0, 0);
    ASSERT_NEAR(e.y, 0, 1e-3, "equator ECEF y");
    ASSERT(e.x > 6e6 && e.x < 6.5e6, "equator ECEF x");

    Geodetic g = conv.ecefToGeodetic(e.x, e.y, e.z);
    ASSERT_NEAR(g.latitude, 0, 1e-6, "ECEF roundtrip lat");
    ASSERT_NEAR(g.longitude, 0, 1e-6, "ECEF roundtrip lon");

    conv.setOrigin(CoordinateConverter::degToRad(-23.55), CoordinateConverter::degToRad(-46.63), 800);
    ENU enu = conv.geodeticToENU(conv.getOrigin().latitude, conv.getOrigin().longitude, conv.getOrigin().altitude);
    ASSERT_NEAR(enu.east, 0, 1e-3, "origin ENU east");
    ASSERT_NEAR(enu.north, 0, 1e-3, "origin ENU north");
}

static void testMatrix() {
    using namespace engine::math;

    Matrix<3, 3> A;
    A.setIdentity();
    ASSERT_NEAR(A(0, 0), 1.0, 1e-9, "identity");
    ASSERT_NEAR(A(1, 1), 1.0, 1e-9, "identity");

    Matrix<3, 3> B = A;
    Matrix<3, 3> C = A * B;
    ASSERT_NEAR(C(0, 0), 1.0, 1e-9, "I*I");

    Matrix<3, 3> D;
    D(0, 0) = 2; D(0, 1) = 0; D(0, 2) = 0;
    D(1, 0) = 0; D(1, 1) = 2; D(1, 2) = 0;
    D(2, 0) = 0; D(2, 1) = 0; D(2, 2) = 2;
    Matrix<3, 3> Dinv;
    ASSERT(invert(D, Dinv), "invert 2I");
    ASSERT_NEAR(Dinv(0, 0), 0.5, 1e-9, "inv diagonal");
}

int main() {
    testQuaternion();
    testCoordinateConverter();
    testMatrix();

    std::cout << (g_fail == 0 ? "EKF math tests OK" : "FAILED") << std::endl;
    return g_fail;
}
