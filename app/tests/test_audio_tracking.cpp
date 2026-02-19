/**
 * Unit tests for AudioTrackingIntegration (voice command parsing, TTS feedback).
 */
#include "app/audio/AudioTrackingIntegration.hpp"
#include "app/audio/CallbackTrackingController.hpp"
#include "engine/audio/voice_agent/IVoiceAgent.hpp"
#include "engine/audio/tts/ITextToSpeech.hpp"
#include <iostream>
#include <cassert>

using namespace app::audio;

class StubVoiceAgent : public engine::audio::IVoiceAgent {
public:
    void startListening() override {}
    void stopListening() override {}
    void setOnCommand(OnCommand cb) override { onCommand_ = cb; }
    void fireCommand(const std::string& cmd) { if (onCommand_) onCommand_(cmd); }
    OnCommand onCommand_;
};

class StubTTS : public engine::audio::ITextToSpeech {
public:
    void speak(const std::string& text) override { lastSpoken_ = text; }
    void setLanguage(const std::string&) override {}
    void setVoice(const std::string&) override {}
    std::string lastSpoken_;
};

static void test_normalize() {
    assert(AudioTrackingIntegration::normalizeCommand("  START  TRACKING  ") == "start tracking");
    assert(AudioTrackingIntegration::normalizeCommand("stop_tracking") == "stop tracking");
    std::cout << "normalize: OK" << std::endl;
}

static void test_command_matching() {
    assert(AudioTrackingIntegration::isStartTracking("start tracking"));
    assert(AudioTrackingIntegration::isStartTracking("start_tracking"));
    assert(AudioTrackingIntegration::isStartTracking("START TRACKING"));
    assert(!AudioTrackingIntegration::isStartTracking("stop tracking"));
    assert(AudioTrackingIntegration::isStopTracking("stop tracking"));
    assert(AudioTrackingIntegration::isStopTracking("stop_tracking"));
    assert(!AudioTrackingIntegration::isStopTracking("start tracking"));
    std::cout << "command matching: OK" << std::endl;
}

static void test_voice_to_start_stop() {
    auto agent = std::make_unique<StubVoiceAgent>();
    auto ctrl = std::make_unique<CallbackTrackingController>();
    int startCount = 0, stopCount = 0;
    ctrl->setOnStart([&]() { startCount++; });
    ctrl->setOnStop([&]() { stopCount++; });

    AudioTrackingIntegration integration;
    integration.setVoiceAgent(agent.get());
    integration.setTrackingController(ctrl.get());

    agent->fireCommand("start tracking");
    assert(startCount == 1);

    agent->fireCommand("stop_tracking");
    assert(stopCount == 1);

    agent->fireCommand("unknown command");
    assert(startCount == 1 && stopCount == 1);

    std::cout << "voice to start/stop: OK" << std::endl;
}

static void test_tts_feedback() {
    auto tts = std::make_unique<StubTTS>();
    AudioTrackingIntegration integration;
    integration.setTTS(tts.get());

    integration.speakFeedback(12.3, 1.24, "km/h", "km");
    assert(tts->lastSpoken_.find("12.3") != std::string::npos);
    assert(tts->lastSpoken_.find("1.2") != std::string::npos);
    assert(tts->lastSpoken_.find("km/h") != std::string::npos);

    std::cout << "TTS feedback: OK" << std::endl;
}

int main() {
    test_normalize();
    test_command_matching();
    test_voice_to_start_stop();
    test_tts_feedback();
    std::cout << "All audio-tracking tests passed." << std::endl;
    return 0;
}
