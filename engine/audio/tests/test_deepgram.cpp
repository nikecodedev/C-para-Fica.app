/**
 * Unit tests for Deepgram TTS and Voice Agent.
 * Uses stub transports; no network I/O.
 */
#include "../AudioEngine.hpp"
#include "../tts/DeepgramTTS.hpp"
#include "../voice_agent/DeepgramVoiceAgent.hpp"
#include "stub_transports.hpp"
#include "FileAudioSource.hpp"
#include "AudioTestFixtures.hpp"
#include <iostream>
#include <cassert>
#include <cstring>
#include <memory>
#include <thread>
#include <chrono>

using namespace engine::audio;

// -----------------------------------------------------------------------------
// TTS playback tests
// -----------------------------------------------------------------------------

static void test_tts_playback_basic() {
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

    std::cout << "TTS playback basic: OK" << std::endl;
}

static void test_tts_empty_text_no_request() {
    auto http = std::make_shared<StubHttpTransport>();
    auto sink = std::make_shared<StubAudioSink>();
    AudioConfig config;
    config.apiKey = "test-key";

    DeepgramTTS tts(http, sink, config);
    tts.speak("");
    assert(http->postCount_ == 0);
    assert(sink->playCount_ == 0);
    std::cout << "TTS empty text: OK" << std::endl;
}

static void test_tts_api_failure_no_playback() {
    auto http = std::make_shared<StubHttpTransport>();
    auto sink = std::make_shared<StubAudioSink>();
    AudioConfig config;
    config.apiKey = "test-key";

    DeepgramTTS tts(http, sink, config);
    http->success_ = false;
    http->statusCode_ = 500;
    http->responseBody_ = {};
    tts.speak("fail");

    assert(http->postCount_ == 1);
    assert(sink->playCount_ == 0);
    std::cout << "TTS API failure: OK" << std::endl;
}

static void test_tts_url_format() {
    auto http = std::make_shared<StubHttpTransport>();
    auto sink = std::make_shared<StubAudioSink>();
    AudioConfig config;
    config.apiKey = "key";
    config.deepgram.ttsVoice = "aura-luna-pt";
    config.deepgram.ttsSampleRate = 24000;

    DeepgramTTS tts(http, sink, config);
    tts.speak("x");
    assert(http->postCount_ == 1);
    assert(http->lastBody_.size() > 0);
    std::cout << "TTS URL format: OK" << std::endl;
}

// -----------------------------------------------------------------------------
// Voice command transcription tests
// -----------------------------------------------------------------------------

static void test_voice_command_final_transcript() {
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
    std::cout << "Voice command final: OK" << std::endl;
}

static void test_voice_command_is_final() {
    auto ws = std::make_shared<StubWebSocketTransport>();
    auto source = std::make_shared<StubAudioSource>();
    auto timer = std::make_shared<StubTimer>();
    AudioConfig config;
    config.apiKey = "test-key";

    DeepgramVoiceAgent agent(ws, source, timer, config);
    std::string cmd;
    agent.setOnCommand([&cmd](const std::string& c) { cmd = c; });
    agent.startListening();

    const char* json = R"({"channel":{"alternatives":[{"transcript":"stop"}]},"is_final":true})";
    if (ws->onText_) ws->onText_(json);
    assert(cmd == "stop");

    agent.stopListening();
    std::cout << "Voice command is_final: OK" << std::endl;
}

static void test_voice_interim_no_callback() {
    auto ws = std::make_shared<StubWebSocketTransport>();
    auto source = std::make_shared<StubAudioSource>();
    auto timer = std::make_shared<StubTimer>();
    AudioConfig config;
    config.apiKey = "test-key";

    DeepgramVoiceAgent agent(ws, source, timer, config);
    int callbackCount = 0;
    agent.setOnCommand([&callbackCount](const std::string&) { callbackCount++; });
    agent.startListening();

    const char* interim = R"({"channel":{"alternatives":[{"transcript":"partial"}]},"speech_final":false})";
    if (ws->onText_) ws->onText_(interim);
    assert(callbackCount == 0);

    agent.stopListening();
    std::cout << "Voice interim no callback: OK" << std::endl;
}

static void test_voice_transcript_with_spaces() {
    auto ws = std::make_shared<StubWebSocketTransport>();
    auto source = std::make_shared<StubAudioSource>();
    auto timer = std::make_shared<StubTimer>();
    AudioConfig config;
    config.apiKey = "test-key";

    DeepgramVoiceAgent agent(ws, source, timer, config);
    std::string cmd;
    agent.setOnCommand([&cmd](const std::string& c) { cmd = c; });
    agent.startListening();

    const char* json = R"({"channel":{"alternatives":[{"transcript": "turn left"}]},"speech_final":true})";
    if (ws->onText_) ws->onText_(json);
    assert(cmd == "turn left");

    agent.stopListening();
    std::cout << "Voice transcript spaces: OK" << std::endl;
}

// -----------------------------------------------------------------------------
// Replay recorded audio tests
// -----------------------------------------------------------------------------

static void test_replay_audio_file() {
    const std::string path = "test_replay_audio.raw";
    if (!tests::writeTestPcmFile(path, 16000, 0.2f)) {
        std::cerr << "Could not write test PCM file\n";
        return;
    }

    auto ws = std::make_shared<StubWebSocketTransport>();
    auto timer = std::make_shared<StubTimer>();
    AudioConfig config;
    config.apiKey = "test-key";
    config.sampleRate = 16000;

    auto fileSourcePtr = std::make_shared<tests::FileAudioSource>(path, 16000);
    fileSourcePtr->setChunkSize(1024);

    DeepgramVoiceAgent agent(ws, fileSourcePtr, timer, config);
    agent.startListening();

    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    fileSourcePtr->stopCapture();

    assert(ws->binarySendCount_ > 0);
    assert(ws->totalBytesSent_ > 0);

    agent.stopListening();
    std::cout << "Replay audio file: OK" << std::endl;
}

int main() {
    test_tts_playback_basic();
    test_tts_empty_text_no_request();
    test_tts_api_failure_no_playback();
    test_tts_url_format();

    test_voice_command_final_transcript();
    test_voice_command_is_final();
    test_voice_interim_no_callback();
    test_voice_transcript_with_spaces();

    test_replay_audio_file();

    std::cout << "All Deepgram tests passed." << std::endl;
    return 0;
}
