#pragma once

#include <string>
#include <vector>
#include <functional>

namespace engine {
namespace audio {

/**
 * Text-to-Speech interface. Platform implements via Deepgram API.
 * No iOS/Android SDK calls here—platform layer does HTTP/WebSocket.
 */
class ITextToSpeech {
public:
    virtual ~ITextToSpeech() = default;

    /** Callback: (success, audio_data). audio_data is PCM bytes. */
    using OnComplete = std::function<void(bool success, const std::vector<uint8_t>& audioData)>;

    /**
     * Synthesize text to audio. Async; result via callback.
     * @param text Text to speak
     * @param callback Called when done (success, PCM bytes)
     */
    virtual void synthesize(const std::string& text, OnComplete callback) = 0;

    /** Cancel any in-flight synthesis. */
    virtual void cancel() = 0;

    /** Whether currently synthesizing. */
    virtual bool isBusy() const = 0;
};

}  // namespace audio
}  // namespace engine
