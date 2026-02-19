#include "FileAudioSource.hpp"
#include <fstream>
#include <thread>

namespace engine {
namespace audio {
namespace tests {

void FileAudioSource::startCapture(OnAudioData onData) {
    if (!onData) return;
    running_ = true;
    std::thread([this, onData]() {
        std::ifstream f(path_, std::ios::binary);
        if (!f) {
            running_ = false;
            return;
        }
        std::vector<std::uint8_t> buf(chunkSize_);
        while (running_.load() && f) {
            f.read(reinterpret_cast<char*>(buf.data()), static_cast<std::streamsize>(chunkSize_));
            size_t n = static_cast<size_t>(f.gcount());
            if (n == 0) break;
            buf.resize(n);
            onData(buf);
        }
        running_ = false;
    }).detach();
}

void FileAudioSource::stopCapture() {
    running_.store(false);
}

}  // namespace tests
}  // namespace audio
}  // namespace engine
