#include "../core/EngineAccessManager.hpp"
#include <iostream>

int main() {
    engine::core::EngineAccessManager accessManager;

    accessManager.getEngine().setOrigin(-23.5505, -46.6333, 800.0);

    double t = 0.0;

    accessManager.setSubscriptionActive(false);
    for (int i = 0; i < 5; ++i) {
        accessManager.getEngine().pushGps(t, -23.5505 + i * 0.0001, -46.6333, 800.0);
        auto state = accessManager.tick(t);
        std::cout << "GPS-only t=" << t << " pos=" << state.px << "," << state.py << "," << state.pz << std::endl;
        t += 0.01;
    }

    accessManager.setSubscriptionActive(true);
    for (int i = 0; i < 10; ++i) {
        accessManager.getEngine().pushImu(t, 0, 0, 9.81, 0, 0, 0);
        accessManager.getEngine().pushGps(t, -23.5505, -46.6333, 800.0);
        auto state = accessManager.tick(t);
        std::cout << "100Hz t=" << t << " pos=" << state.px << "," << state.py << "," << state.pz << std::endl;
        t += 0.01;
    }

    std::cout << "EngineAccessManager test OK" << std::endl;
    return 0;
}
