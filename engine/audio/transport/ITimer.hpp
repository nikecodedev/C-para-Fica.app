#pragma once

#include <functional>
#include <cstdint>

namespace engine {
namespace audio {

/**
 * Platform-agnostic timer for periodic tasks (e.g. KeepAlive).
 * Platform implements using Grand Central Dispatch, Handler, etc.
 */
class ITimer {
public:
    virtual ~ITimer() = default;

    /** Start recurring callback every intervalMs. Replaces any existing schedule. */
    virtual void start(std::uint32_t intervalMs, std::function<void()> callback) = 0;

    /** Stop the timer. */
    virtual void stop() = 0;
};

}  // namespace audio
}  // namespace engine
