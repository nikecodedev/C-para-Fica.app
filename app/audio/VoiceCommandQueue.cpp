#include "VoiceCommandQueue.hpp"

namespace app {
namespace audio {

void VoiceCommandQueue::push(const std::string& cmd) {
    std::lock_guard<std::mutex> lock(mutex_);
    queue_.push(cmd);
}

void VoiceCommandQueue::drain(const std::function<void(const std::string&)>& handler) {
    std::queue<std::string> copy;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        copy.swap(queue_);
    }
    while (!copy.empty()) {
        handler(copy.front());
        copy.pop();
    }
}

size_t VoiceCommandQueue::size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.size();
}

}  // namespace audio
}  // namespace app
