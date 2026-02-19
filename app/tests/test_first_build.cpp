/**
 * First build integration test. Verifies pipeline: engine, EKF, logging, 100Hz.
 */
#include "app/ui/FirstBuildIntegration.hpp"
#include <iostream>

int main() {
    app::ui::FirstBuildIntegration integration;

    integration.getEngine().getEngine().setOrigin(-23.55, -46.63, 800.0);
    integration.setSubscriptionActive(true);

    for (int i = 0; i < 200; ++i) {
        double t = i * 0.01;
        integration.getEngine().getEngine().pushImu(t, 0, 0, 9.81, 0, 0, 0);
        integration.getEngine().getEngine().pushGps(t, -23.55 + t * 0.0001, -46.63, 800);
        integration.getEngine().getEngine().pushMag(t, 0, 1, 0);
        integration.tick(t);
    }

    std::cout << "Speed: " << integration.getSpeedDisplay() << " " << integration.getSpeedUnit() << std::endl;
    std::cout << "Distance: " << integration.getDistanceDisplay() << " " << integration.getDistanceUnit() << std::endl;
    std::cout << "Trajectory points: " << integration.getTrajectory().size() << std::endl;

    bool ok = integration.exportLog("first_build_test.csv");
    std::cout << (ok ? "First build pipeline OK" : "Log export failed") << std::endl;

    return ok ? 0 : 1;
}
