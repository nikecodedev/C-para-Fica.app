#include "DeepgramVoiceAgent.hpp"
#include "../logging/AudioLogger.hpp"
#include <sstream>
#include <algorithm>
#include <cassert>
#include <cstring>

namespace engine {
namespace audio {

namespace {

constexpr const char* DEEPGRAM_WS_BASE = "wss://api.deepgram.com/v1/listen";

}  // namespace

DeepgramVoiceAgent::DeepgramVoiceAgent(std::shared_ptr<IWebSocketTransport> ws,
                                        std::shared_ptr<IAudioSource> audioSource,
                                        std::shared_ptr<ITimer> timer,
                                        const AudioConfig& config)
    : ws_(std::move(ws))
    , audioSource_(std::move(audioSource))
    , timer_(std::move(timer))
    , config_(config)
    , log_(nullptr)
{
    assert(ws_ && audioSource_ && timer_);
}

DeepgramVoiceAgent::~DeepgramVoiceAgent() {
    stopListening();
}

void DeepgramVoiceAgent::startListening() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (listening_) return;
    listening_ = true;

    ws_->setOnText([this](const std::string& text) {
        onTextMessage(text);
    });
    ws_->setOnBinary([](const std::vector<std::uint8_t>&) {
        /* Deepgram typically sends text JSON; ignore binary */
    });
    ws_->setOnOpen([this]() {
        log_.logStreamEvent("opened");
        timer_->start(KEEPALIVE_MS, [this]() { sendKeepAlive(); });
        audioSource_->startCapture([this](const std::vector<std::uint8_t>& chunk) {
            onAudioChunk(chunk);
        });
    });
    ws_->setOnClose([this](const std::string& reason) {
        timer_->stop();
        audioSource_->stopCapture();
        std::lock_guard<std::mutex> lock(mutex_);
        listening_ = false;
        log_.logStreamEvent("closed", reason);
    });

    connectWebSocket();
}

void DeepgramVoiceAgent::stopListening() {
    timer_->stop();
    audioSource_->stopCapture();
    ws_->close();
    std::lock_guard<std::mutex> lock(mutex_);
    listening_ = false;
}

void DeepgramVoiceAgent::setOnCommand(OnCommand callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    onCommand_ = std::move(callback);
}

void DeepgramVoiceAgent::setConfig(const AudioConfig& config) {
    config_ = config;
}

void DeepgramVoiceAgent::connectWebSocket() {
    std::string key = config_.deepgram.apiKey.empty() ? config_.apiKey : config_.deepgram.apiKey;
    if (key.empty()) {
        if (IAudioLogger* L = log_.getDelegate())
            L->log(IAudioLogger::Level::Error, "DeepgramVoiceAgent: API key not set");
        return;
    }

    int sampleRate = config_.deepgram.sttSampleRate > 0
        ? config_.deepgram.sttSampleRate
        : (config_.sampleRate > 0 ? config_.sampleRate : 16000);
    std::ostringstream oss;
    bool interim = config_.deepgram.useInterimResults();
    int endMs = config_.deepgram.utteranceEndMs();
    int sr = config_.deepgram.sttSampleRate > 0 ? config_.deepgram.sttSampleRate : sampleRate;

    oss << DEEPGRAM_WS_BASE
        << "?encoding=linear16"
        << "&sample_rate=" << sr
        << "&language=" << (config_.deepgram.language.empty() ? config_.language : config_.deepgram.language)
        << "&interim_results=" << (interim ? "true" : "false")
        << "&punctuate=true"
        << "&smart_format=true"
        << "&utterance_end_ms=" << endMs;

    std::vector<std::pair<std::string, std::string>> headers = {
        {"Authorization", "Token " + key}
    };

    ws_->connect(oss.str(), headers);
}

void DeepgramVoiceAgent::onTextMessage(const std::string& text) {
    std::string transcript = extractTranscript(text);
    if (transcript.empty()) return;

    if (extractSpeechFinal(text)) {
        log_.logVoiceCommand(transcript);
        log_.logApiResponse(true, 200, text.size());
        OnCommand cb;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            cb = onCommand_;
        }
        if (cb) cb(transcript);
    }
}

void DeepgramVoiceAgent::onAudioChunk(const std::vector<std::uint8_t>& chunk) {
    if (ws_->isConnected() && !chunk.empty()) {
        ws_->sendBinary(chunk);
    }
}

void DeepgramVoiceAgent::sendKeepAlive() {
    if (ws_->isConnected()) {
        ws_->sendText(R"({"type":"KeepAlive"})");
    }
}

std::string DeepgramVoiceAgent::extractTranscript(const std::string& json) const {
    for (const char* needle : {"\"transcript\":\"", "\"transcript\": \""}) {
        auto pos = json.find(needle);
        if (pos != std::string::npos) {
            pos += std::strlen(needle);
            auto end = json.find('"', pos);
            if (end != std::string::npos) {
                std::string transcript = json.substr(pos, end - pos);
                if (!transcript.empty()) return transcript;
            }
        }
    }
    return {};
}

bool DeepgramVoiceAgent::extractSpeechFinal(const std::string& json) const {
    return json.find("\"speech_final\":true") != std::string::npos
        || json.find("\"is_final\":true") != std::string::npos;
}

}  // namespace audio
}  // namespace engine
