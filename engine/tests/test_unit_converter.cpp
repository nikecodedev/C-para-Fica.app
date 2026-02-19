#include "../config/UnitConverter.hpp"
#include <iostream>
#include <cmath>

int main() {
    engine::config::UnitConverter conv;

    conv.setUnitSystem(engine::config::UnitSystem::Metric);
    double km = conv.distanceToDisplay(1000.0);
    double kmh = conv.speedToDisplay(10.0);
    bool ok = (std::abs(km - 1.0) < 1e-6 && std::abs(kmh - 36.0) < 1e-6);

    conv.setUnitSystem(engine::config::UnitSystem::Imperial);
    double mi = conv.distanceToDisplay(1609.344);
    double mph = conv.speedToDisplay(1.0);
    ok = ok && (std::abs(mi - 1.0) < 1e-4 && std::abs(mph - 2.237) < 0.01);

    conv.setFromRegion("US");
    ok = ok && (conv.getUnitSystem() == engine::config::UnitSystem::Imperial);

    conv.setFromRegion("BR");
    ok = ok && (conv.getUnitSystem() == engine::config::UnitSystem::Metric);

    std::cout << (ok ? "UnitConverter OK" : "FAIL") << std::endl;
    return ok ? 0 : 1;
}
