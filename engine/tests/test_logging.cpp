#include "../core/TrackingEngine.hpp"
#include "../logging/TrackingLogger.hpp"
#include <iostream>

int main() {
    engine::core::TrackingEngine engine;
    engine::logging::TrackingLogger logger;

    engine.setLogger(&logger);
    engine.setOrigin(-23.5505, -46.6333, 800.0);

    double t = 0.0;
    for (int i = 0; i < 100; ++i) {
        engine.pushImu(t, 0, 0, 9.81, 0, 0, 0);
        if (i % 10 == 0) {
            engine.pushGps(t, -23.5505, -46.6333, 800.0);
        }
        engine.pushMag(t, 0, 1, 0);

        engine.tick(t);
        t += 0.01;
    }

    const bool ok = logger.exportToCsv("tracking_log.csv");
    std::cout << "Logging: " << logger.size() << " entries, CSV export "
              << (ok ? "OK" : "FAILED") << std::endl;

    return ok ? 0 : 1;
}
