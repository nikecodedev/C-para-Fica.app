#include "../core/TrackingEngine.hpp"
#include <iostream>
#include <cmath>

int main() {
    engine::core::TrackingEngine engine;

    engine.setOrigin(-23.5505, -46.6333, 800.0);

    double t = 0.0;
    engine::core::TrackingState state;
    for (int i = 0; i < 200; ++i) {
        engine.pushImu(t, 0, 0, 9.81, 0, 0, 0);
        if (i % 10 == 0) {
            engine.pushGps(t, -23.5505, -46.6333, 800.0 + t * 0.1);
        }
        engine.pushMag(t, 0, 1, 0);

        state = engine.tick(t);

        t += 0.01;
    }

    std::cout << "TrackingEngine 100Hz loop OK. Final position: "
              << state.px << ", " << state.py << ", " << state.pz << std::endl;

    return 0;
}
