#pragma once

#include <vector>
#include <cstdint>

namespace engine {
namespace audio {

/**
 * Platform-agnostic audio output sink for playback.
 * Platform implements using AVAudioEngine (iOS), AudioTrack (Android).
 */
class IAudioSink {
public:
    virtual ~IAudioSink() = default;

    /** Play PCM audio. Format: linear16, mono, sample rate from config. */
    virtual void play(const std::vector<std::uint8_t>& pcmData) = 0;

    /** Stop playback. */
    virtual void stop() = 0;

    /** Sample rate this sink expects (e.g. 16000, 24000). */
    virtual int getSampleRate() const = 0;
};

}  // namespace audio
}  // namespace engine
