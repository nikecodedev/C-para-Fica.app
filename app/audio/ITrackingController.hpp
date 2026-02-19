#pragma once

namespace app {
namespace audio {

/**
 * Callback interface for voice-driven tracking control.
 * Platform implements: start_tracking → SensorBridge.start() + subscription;
 * stop_tracking → SensorBridge.stop() + subscription off.
 */
class ITrackingController {
public:
    virtual ~ITrackingController() = default;

    virtual void startTracking() = 0;
    virtual void stopTracking() = 0;
};

}  // namespace audio
}  // namespace app
