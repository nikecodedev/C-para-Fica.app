/**
 * Smoke test for audio interfaces. No platform SDK calls.
 */
#include "../AudioEngine.hpp"
#include "../tts/ITextToSpeech.hpp"
#include "../voice_agent/IVoiceAgent.hpp"
#include <iostream>

class StubTTS : public engine::audio::ITextToSpeech {
public:
    void synthesize(const std::string&, OnComplete callback) override {
        std::vector<uint8_t> empty;
        callback(true, empty);
    }
    void cancel() override {}
    bool isBusy() const override { return false; }
};

class StubVoiceAgent : public engine::audio::IVoiceAgent {
public:
    void startListening(OnTranscript /*onTranscript*/, OnError /*onError*/) override {}
    void stopListening() override {}
    bool isListening() const override { return false; }
};

int main() {
    engine::audio::AudioEngine engine;
    engine.setTTS(std::make_unique<StubTTS>());
    engine.setVoiceAgent(std::make_unique<StubVoiceAgent>());

    bool ok = (engine.getTTS() != nullptr && engine.getVoiceAgent() != nullptr);
    std::cout << (ok ? "Audio interfaces OK" : "FAIL") << std::endl;
    return ok ? 0 : 1;
}
