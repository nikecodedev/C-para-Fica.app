#pragma once

/** IAudioSource that replays PCM from file. For testing. */

#include "../transport/IAudioSource.hpp"
#include <atomic>
#include <string>
#include <fstream>

namespace engine {
namespace audio {
namespace tests {

/**
 * Replay audio from raw PCM file. Format: linear16, mono.
 * Used for reproducible voice-agent testing without live microphone.
 */
class FileAudioSource : public IAudioSource {
public:
    explicit FileAudioSource(const std::string& path, int sampleRate = 16000)
        : path_(path), sampleRate_(sampleRate) {}

    void startCapture(OnAudioData onData) override;
    void stopCapture() override;
    int getSampleRate() const override { return sampleRate_; }

    /** Chunk size in bytes (default 2048). */
    void setChunkSize(size_t bytes) { chunkSize_ = bytes; }

private:
    std::string path_;
    int sampleRate_;
    size_t chunkSize_{2048};
    std::atomic<bool> running_{false};
};

}  // namespace tests
}  // namespace audio
}  // namespace engine
