/**
 * Smoke test for audio interfaces. No platform SDK calls.
 */
#include "../AudioEngine.hpp"
#include "../tts/ITextToSpeech.hpp"
#include "../voice_agent/IVoiceAgent.hpp"
#include <iostream>

class StubTTS : public engine::audio::ITextToSpeech {
public:
    void speak(const std::string&) override {}
    void setLanguage(const std::string&) override {}
    void setVoice(const std::string&) override {}
};

class StubVoiceAgent : public engine::audio::IVoiceAgent {
public:
    void startListening() override {}
    void stopListening() override {}
    void setOnCommand(OnCommand) override {}
};

int main() {
    engine::audio::AudioEngine engine;
    engine.setTTS(std::make_unique<StubTTS>());
    engine.setVoiceAgent(std::make_unique<StubVoiceAgent>());

    bool ok = (engine.getTTS() != nullptr && engine.getVoiceAgent() != nullptr);
    std::cout << (ok ? "Audio interfaces OK" : "FAIL") << std::endl;
    return ok ? 0 : 1;
}
