#include "SensorReplay.hpp"
#include "../math/CoordinateConverter.hpp"
#include <fstream>
#include <sstream>

namespace engine {
namespace tests {

SensorReplay::SensorReplay(core::EngineAccessManager& engine) : engine_(engine) {}

bool SensorReplay::load(const std::string& csvPath) {
    std::ifstream f(csvPath);
    if (!f) return false;
    std::string line;
    std::getline(f, line);
    rows_.clear();
    while (std::getline(f, line)) {
        std::vector<double> row;
        std::istringstream ss(line);
        std::string cell;
        while (std::getline(ss, cell, ',')) {
            try { row.push_back(std::stod(cell)); } catch (...) { row.push_back(0); }
        }
        if (row.size() >= 42) rows_.push_back(std::move(row));
    }
    rowIdx_ = 0;
    return !rows_.empty();
}

bool SensorReplay::step(core::TrackingState& stateOut) {
    if (rowIdx_ >= rows_.size()) return false;
    const auto& vals = rows_[rowIdx_++];

    double t = vals[0];
    if (vals[33] > 0.5)
        engine_.getEngine().pushImu(t, vals[27], vals[28], vals[29], vals[30], vals[31], vals[32]);
    if (vals[37] > 0.5) {
        auto& conv = engine_.getEngine().getCoordinateConverter();
        math::Geodetic g = conv.enuToGeodetic(vals[34], vals[35], vals[36]);
        engine_.getEngine().pushGps(t, math::CoordinateConverter::radToDeg(g.latitude),
                                    math::CoordinateConverter::radToDeg(g.longitude), g.altitude);
    }
    if (vals[41] > 0.5)
        engine_.getEngine().pushMag(t, vals[38], vals[39], vals[40]);

    engine_.setSubscriptionActive(true);
    stateOut = engine_.tick(t);
    return true;
}


}  // namespace tests
}  // namespace engine
