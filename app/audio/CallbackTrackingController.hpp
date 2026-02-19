#pragma once

#include "ITrackingController.hpp"
#include <functional>

namespace app {
namespace audio {

/**
 * ITrackingController implementation using callbacks.
 * Platform sets callbacks (e.g. JNI to SensorBridge.start/stop).
 */
class CallbackTrackingController : public ITrackingController {
public:
    void setOnStart(std::function<void()> cb) { onStart_ = std::move(cb); }
    void setOnStop(std::function<void()> cb) { onStop_ = std::move(cb); }

    void startTracking() override { if (onStart_) onStart_(); }
    void stopTracking() override { if (onStop_) onStop_(); }

private:
    std::function<void()> onStart_;
    std::function<void()> onStop_;
};

}  // namespace audio
}  // namespace app
