#pragma once

#include <string>
#include <queue>
#include <mutex>
#include <functional>

namespace app {
namespace audio {

/**
 * Thread-safe queue for voice commands. Voice callback pushes and returns immediately;
 * platform drains on main/tick thread to avoid blocking the 100Hz loop or network threads.
 */
class VoiceCommandQueue {
public:
    /** Push command from any thread (e.g. WebSocket callback). Non-blocking. */
    void push(const std::string& cmd);

    /** Drain all queued commands, invoking handler for each. Call from main/tick thread. */
    void drain(const std::function<void(const std::string&)>& handler);

    /** Number of commands currently queued. */
    size_t size() const;

private:
    mutable std::mutex mutex_;
    std::queue<std::string> queue_;
};

}  // namespace audio
}  // namespace app
