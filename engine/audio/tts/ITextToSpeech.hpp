#pragma once

#include <string>

namespace engine {
namespace audio {

/**
 * Text-to-Speech interface. Platform implements via Deepgram API.
 * Supports PT/EN and voice selection. No iOS/Android SDK calls here.
 */
class ITextToSpeech {
public:
    virtual ~ITextToSpeech() = default;

    /** Speak the given text. Blocks or queues; platform plays audio. */
    virtual void speak(const std::string& text) = 0;

    /** Set language code (e.g. "en", "pt"). */
    virtual void setLanguage(const std::string& lang) = 0;

    /** Set voice ID (e.g. "aura-asteria-en", "aura-luna-pt"). */
    virtual void setVoice(const std::string& voice) = 0;
};

}  // namespace audio
}  // namespace engine
