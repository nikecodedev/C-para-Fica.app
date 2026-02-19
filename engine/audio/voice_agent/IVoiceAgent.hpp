#pragma once

#include <string>
#include <functional>

namespace engine {
namespace audio {

/**
 * Voice Agent interface. Captures device audio, sends to Deepgram for transcription.
 * Returns command string to engine via onCommand callback. No iOS/Android SDK calls here.
 */
class IVoiceAgent {
public:
    virtual ~IVoiceAgent() = default;

    /** Callback: transcribed command from user speech. Engine registers via setOnCommand. */
    using OnCommand = std::function<void(const std::string& command)>;

    /** Start listening. Platform captures mic, streams to Deepgram API. */
    virtual void startListening() = 0;

    /** Stop listening. */
    virtual void stopListening() = 0;

    /** Register callback to receive transcribed commands. */
    virtual void setOnCommand(OnCommand callback) = 0;
};

}  // namespace audio
}  // namespace engine
