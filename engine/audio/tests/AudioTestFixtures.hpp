#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <cstdint>

namespace engine {
namespace audio {
namespace tests {

/** Create a minimal raw PCM file (linear16, mono) for replay testing. */
inline bool writeTestPcmFile(const std::string& path, int sampleRate = 16000, float durationSec = 0.1f) {
    size_t numSamples = static_cast<size_t>(sampleRate * durationSec);
    std::vector<std::int16_t> samples(numSamples, 0);
    std::ofstream f(path, std::ios::binary);
    if (!f) return false;
    f.write(reinterpret_cast<const char*>(samples.data()), static_cast<std::streamsize>(samples.size() * sizeof(std::int16_t)));
    return f.good();
}

/** Get bytes for 100ms of silence (16kHz, 16-bit mono). */
inline std::vector<std::uint8_t> makeSilenceChunk(int sampleRate = 16000, float durationSec = 0.1f) {
    size_t numSamples = static_cast<size_t>(sampleRate * durationSec);
    std::vector<std::uint8_t> out(numSamples * 2, 0);
    return out;
}

}  // namespace tests
}  // namespace audio
}  // namespace engine
