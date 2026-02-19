#pragma once

#include <string>

namespace engine {
namespace audio {

/**
 * Audio module logging. Platform-agnostic interface.
 * Platform layer can implement file, console, or remote logging.
 */
class IAudioLogger {
public:
    virtual ~IAudioLogger() = default;

    enum class Level { Debug, Info, Warning, Error };

    virtual void log(Level level, const std::string& message) = 0;
};

/** No-op logger for production when logging disabled. */
class NullAudioLogger : public IAudioLogger {
public:
    void log(Level, const std::string&) override {}
};

}  // namespace audio
}  // namespace engine
