#include "../fusion/ExtendedKalmanFilter.hpp"
#include "../fusion/EKFConfig.hpp"
#include "../math/Quaternion.hpp"
#include <iostream>
#include <cmath>

int main() {
    engine::fusion::ExtendedKalmanFilter ekf;
    engine::fusion::EKFConfig config;
    ekf.setConfig(config);

    ekf.setState(0, 0, 0, 0, 0, 0,
                 engine::math::Quaternion::identity(),
                 0, 0, 0, 0, 0, 0);

    ekf.setImuInput(0, 0, 9.81, 0, 0, 0);
    ekf.predict(0.01);

    ekf.updateGPS(0.1, 0.2, 0.3);

    const auto& x = ekf.getState();
    std::cout << "Position: " << x(0,0) << ", " << x(1,0) << ", " << x(2,0) << std::endl;

    return 0;
}
