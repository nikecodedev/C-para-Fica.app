#pragma once

#include <vector>
#include <cstdint>

namespace engine {
namespace audio {

/**
 * Platform-agnostic audio output sink for playback.
 * Platform implements using AVAudioEngine (iOS), AudioTrack (Android).
 *
 * Performance: play() must return immediately. Schedule buffer for playback;
 * do not block on actual playback. Must not run on 100Hz tick thread.
 */
class IAudioSink {
public:
    virtual ~IAudioSink() = default;

    /** Play PCM audio. Format: linear16, mono, sample rate from config. Non-blocking. */
    virtual void play(const std::vector<std::uint8_t>& pcmData) = 0;

    /** Stop playback. */
    virtual void stop() = 0;

    /** Sample rate this sink expects (e.g. 16000, 24000). */
    virtual int getSampleRate() const = 0;
};

}  // namespace audio
}  // namespace engine
