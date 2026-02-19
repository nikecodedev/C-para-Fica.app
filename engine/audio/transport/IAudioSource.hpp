#pragma once

#include <vector>
#include <functional>
#include <cstdint>

namespace engine {
namespace audio {

/**
 * Platform-agnostic audio input source (microphone).
 * Platform captures and pushes chunks via onAudioData.
 * Streaming: onAudioData called with small chunks; caller must not block.
 */
class IAudioSource {
public:
    virtual ~IAudioSource() = default;

    /** Callback: raw PCM chunk. Format: linear16, mono, sample rate from config. */
    using OnAudioData = std::function<void(const std::vector<std::uint8_t>& pcmChunk)>;

    /** Start capture. Platform calls onAudioData as data arrives. */
    virtual void startCapture(OnAudioData onData) = 0;

    /** Stop capture. */
    virtual void stopCapture() = 0;

    /** Sample rate of captured audio (e.g. 16000). */
    virtual int getSampleRate() const = 0;
};

}  // namespace audio
}  // namespace engine
