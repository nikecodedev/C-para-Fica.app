#pragma once

#include "../core/TrackingEngine.hpp"
#include "../core/EngineAccessManager.hpp"
#include <string>
#include <vector>

namespace engine {
namespace tests {

/**
 * Replay mode: load CSV log and feed to engine.
 * CSV format: TrackingLogger export. Reproducible fusion testing.
 * Never test fusion blindly in field.
 */
class SensorReplay {
public:
    explicit SensorReplay(core::EngineAccessManager& engine);

    bool load(const std::string& csvPath);
    bool step(core::TrackingState& stateOut);
    bool exhausted() const { return rowIdx_ >= rows_.size(); }

private:
    core::EngineAccessManager& engine_;
    std::vector<std::vector<double>> rows_;
    size_t rowIdx_{0};
};

}  // namespace tests
}  // namespace engine
