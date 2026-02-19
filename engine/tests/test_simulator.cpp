#include "Simulator.hpp"
#include "../core/EngineAccessManager.hpp"
#include <iostream>

int main() {
    engine::core::EngineAccessManager engine;
    engine.getEngine().setOrigin(-23.55, -46.63, 800.0);
    engine.setSubscriptionActive(true);

    engine::tests::Simulator sim;
    sim.setOrigin(-23.55, -46.63, 800.0);
    sim.setTrajectory(5.0, 0, 0);

    for (int i = 0; i < 100; ++i) {
        sim.step(0.01, engine.getEngine());
        auto state = engine.tick(sim.getTime());
    }

    std::cout << "Simulator 100 steps OK" << std::endl;
    return 0;
}
