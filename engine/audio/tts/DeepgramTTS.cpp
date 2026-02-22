#include "DeepgramTTS.hpp"
#include "../logging/AudioLogger.hpp"
#include <sstream>
#include <algorithm>
#include <cassert>

namespace engine {
namespace audio {

namespace {

constexpr const char* DEEPGRAM_TTS_BASE = "https://api.deepgram.com/v1/speak";

std::string escapeJsonString(const std::string& s) {
    std::string out;
    out.reserve(s.size() + 8);
    for (char c : s) {
        switch (c) {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\b': out += "\\b";  break;
            case '\f': out += "\\f";  break;
            case '\n': out += "\\n";  break;
            case '\r': out += "\\r";  break;
            case '\t': out += "\\t";  break;
            default:
                if (static_cast<unsigned char>(c) < 32)
                    out += ' ';  // replace control chars
                else
                    out += c;
        }
    }
    return out;
}

}  // namespace

DeepgramTTS::DeepgramTTS(std::shared_ptr<IHttpTransport> http,
                         std::shared_ptr<IAudioSink> audioSink,
                         const AudioConfig& config)
    : http_(std::move(http))
    , audioSink_(std::move(audioSink))
    , config_(config)
    , log_(nullptr)
{
    assert(http_ && audioSink_);
}

void DeepgramTTS::speak(const std::string& text) {
    if (text.empty()) return;
    std::string key = config_.deepgram.apiKey.empty() ? config_.apiKey : config_.deepgram.apiKey;
    if (key.empty()) {
        if (IAudioLogger* L = log_.getDelegate())
            L->log(IAudioLogger::Level::Error, "DeepgramTTS: API key not set");
        return;
    }

    log_.logTtsRequest(text);
    int startMs = DeepgramLogger::nowMs();

    std::string url = buildRequestUrl();
    std::vector<std::uint8_t> body = buildRequestBody(text);

    std::vector<IHttpTransport::Header> headers = {
        {"Authorization", "Token " + key},
        {"Content-Type", "application/json"}
    };

    http_->post(url, headers, body, [this, startMs](bool success, int statusCode, const std::vector<std::uint8_t>& responseBody) {
        int latencyMs = DeepgramLogger::nowMs() - startMs;
        log_.logApiResponse(success && statusCode == 200, statusCode, responseBody.size(), latencyMs);
        log_.logLatency("TTS request", latencyMs);

        if (!success) return;
        if (statusCode != 200) return;
        if (!responseBody.empty()) {
            audioSink_->play(responseBody);
        }
    });
}

void DeepgramTTS::setLanguage(const std::string& lang) {
    config_.language = lang;
}

void DeepgramTTS::setVoice(const std::string& voice) {
    config_.ttsVoice = voice;
}

std::string DeepgramTTS::buildRequestUrl() const {
    std::string voice = config_.deepgram.ttsVoice.empty() ? config_.ttsVoice : config_.deepgram.ttsVoice;
    int rate = config_.deepgram.ttsSampleRate;
    if (rate != 8000 && rate != 24000)
        rate = (config_.sampleRate == 8000 || config_.sampleRate == 24000) ? config_.sampleRate : 24000;
    std::ostringstream oss;
    oss << DEEPGRAM_TTS_BASE
        << "?model=" << voice
        << "&encoding=linear16"
        << "&sample_rate=" << rate;
    return oss.str();
}

std::vector<std::uint8_t> DeepgramTTS::buildRequestBody(const std::string& text) const {
    std::string json = "{\"text\":\"" + escapeJsonString(text) + "\"}";
    std::vector<std::uint8_t> body(json.begin(), json.end());
    return body;
}

}  // namespace audio
}  // namespace engine
