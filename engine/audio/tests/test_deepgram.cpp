/**
 * Unit tests for Deepgram TTS and Voice Agent.
 * Uses stub transports; no network I/O.
 */
#include "../AudioEngine.hpp"
#include "../tts/DeepgramTTS.hpp"
#include "../voice_agent/DeepgramVoiceAgent.hpp"
#include "stub_transports.hpp"
#include <iostream>
#include <cassert>
#include <cstring>

using namespace engine::audio;

static void test_deepgram_tts() {
    auto http = std::make_shared<StubHttpTransport>();
    auto sink = std::make_shared<StubAudioSink>();
    AudioConfig config;
    config.apiKey = "test-key";
    config.sampleRate = 24000;
    config.ttsVoice = "aura-asteria-en";

    DeepgramTTS tts(http, sink, config);

    assert(sink->playCount_ == 0);
    http->responseBody_ = {0xAB, 0xCD};
    tts.speak("Hello world");

    assert(http->postCount_ == 1);
    std::string body(http->lastBody_.begin(), http->lastBody_.end());
    assert(body.find("\"text\":\"Hello world\"") != std::string::npos ||
           body.find("\"text\": \"Hello world\"") != std::string::npos);

    assert(sink->playCount_ == 1);
    assert(sink->lastData_.size() == 2);
    assert(sink->lastData_[0] == 0xAB && sink->lastData_[1] == 0xCD);

    std::cout << "DeepgramTTS: OK" << std::endl;
}

static void test_deepgram_voice_agent() {
    auto ws = std::make_shared<StubWebSocketTransport>();
    auto source = std::make_shared<StubAudioSource>();
    auto timer = std::make_shared<StubTimer>();
    AudioConfig config;
    config.apiKey = "test-key";
    config.sampleRate = 16000;
    config.language = "en";

    DeepgramVoiceAgent agent(ws, source, timer, config);

    std::string receivedCommand;
    agent.setOnCommand([&receivedCommand](const std::string& cmd) {
        receivedCommand = cmd;
    });

    agent.startListening();

    assert(source->onData_);
    assert(timer->callback_);

    const char* json = R"({"type":"Results","channel":{"alternatives":[{"transcript":"start navigation","confidence":0.9}]},"speech_final":true})";
    if (ws->onText_) ws->onText_(json);

    assert(receivedCommand == "start navigation");

    agent.stopListening();

    std::cout << "DeepgramVoiceAgent: OK" << std::endl;
}

int main() {
    test_deepgram_tts();
    test_deepgram_voice_agent();
    std::cout << "All Deepgram tests passed." << std::endl;
    return 0;
}
