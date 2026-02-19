#include "SensorReplay.hpp"
#include "../core/EngineAccessManager.hpp"
#include "../logging/TrackingLogger.hpp"
#include <iostream>

int main() {
    engine::core::EngineAccessManager engine;
    engine.getEngine().setOrigin(-23.55, -46.63, 800.0);

    engine::logging::TrackingLogger logger;
    engine.getEngine().setLogger(&logger);

    for (int i = 0; i < 50; ++i) {
        double t = i * 0.01;
        engine.getEngine().pushImu(t, 0, 0, 9.81, 0, 0, 0);
        engine.getEngine().pushGps(t, -23.55 + i * 0.0001, -46.63, 800);
        engine.getEngine().pushMag(t, 0, 1, 0);
        engine.tick(t);
    }

    logger.exportToCsv("replay_test.csv");
    engine.getEngine().setLogger(nullptr);

    engine::core::EngineAccessManager replayEngine;
    replayEngine.getEngine().setOrigin(-23.55, -46.63, 800.0);
    engine::tests::SensorReplay replay(replayEngine);
    if (!replay.load("replay_test.csv")) {
        std::cerr << "Replay load failed" << std::endl;
        return 1;
    }

    int count = 0;
    engine::core::TrackingState state;
    while (replay.step(state)) ++count;

    std::cout << "Replay: " << count << " steps OK" << std::endl;
    return 0;
}
