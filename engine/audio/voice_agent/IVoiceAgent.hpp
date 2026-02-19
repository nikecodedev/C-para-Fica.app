#pragma once

#include <string>
#include <functional>

namespace engine {
namespace audio {

/**
 * Voice Agent interface for real-time voice commands.
 * Platform implements via Deepgram streaming API.
 * No iOS/Android SDK calls here—platform layer does WebSocket/audio capture.
 */
class IVoiceAgent {
public:
    virtual ~IVoiceAgent() = default;

    /** Callback: transcribed text from user speech. */
    using OnTranscript = std::function<void(const std::string& text)>;

    /** Callback: error or status. */
    using OnError = std::function<void(const std::string& message)>;

    /**
     * Start listening. Platform captures mic, streams to Deepgram, invokes OnTranscript.
     */
    virtual void startListening(OnTranscript onTranscript, OnError onError) = 0;

    /** Stop listening. */
    virtual void stopListening() = 0;

    /** Whether currently listening. */
    virtual bool isListening() const = 0;
};

}  // namespace audio
}  // namespace engine
