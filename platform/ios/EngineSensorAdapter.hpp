#pragma once

#include "SensorDataSink.hpp"
#include "../../engine/core/EngineAccessManager.hpp"

namespace platform {
namespace ios {

/** Adapter: forwards SensorDataSink calls to EngineAccessManager. */
class EngineSensorAdapter : public SensorDataSink {
public:
    explicit EngineSensorAdapter(engine::core::EngineAccessManager& engine)
        : engine_(engine) {}

    void pushGps(double timestamp, double lat_deg, double lon_deg,
                 double alt_m, double accuracy_m) override {
        engine_.getEngine().pushGps(timestamp, lat_deg, lon_deg, alt_m, accuracy_m);
    }

    void pushImu(double timestamp, double ax, double ay, double az,
                 double gx, double gy, double gz) override {
        engine_.getEngine().pushImu(timestamp, ax, ay, az, gx, gy, gz);
    }

    void pushMag(double timestamp, double mx, double my, double mz) override {
        engine_.getEngine().pushMag(timestamp, mx, my, mz);
    }

private:
    engine::core::EngineAccessManager& engine_;
};

}  // namespace ios
}  // namespace platform
