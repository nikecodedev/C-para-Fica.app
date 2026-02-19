#include "DeepgramTTS.hpp"
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
{
    assert(http_ && audioSink_);
}

void DeepgramTTS::speak(const std::string& text) {
    if (text.empty()) return;
    if (config_.apiKey.empty()) {
        if (logger_) logger_->log(IAudioLogger::Level::Error, "DeepgramTTS: API key not set");
        return;
    }

    std::string url = buildRequestUrl();
    std::vector<std::uint8_t> body = buildRequestBody(text);

    std::vector<IHttpTransport::Header> headers = {
        {"Authorization", "Token " + config_.apiKey},
        {"Content-Type", "application/json"}
    };

    http_->post(url, headers, body, [this, text](bool success, int statusCode, const std::vector<std::uint8_t>& responseBody) {
        if (!success) {
            if (logger_) {
                std::string msg = "DeepgramTTS: HTTP failed status=" + std::to_string(statusCode);
                logger_->log(IAudioLogger::Level::Error, msg);
            }
            return;
        }
        if (statusCode != 200) {
            if (logger_) {
                std::string msg = "DeepgramTTS: API error status=" + std::to_string(statusCode);
                logger_->log(IAudioLogger::Level::Error, msg);
            }
            return;
        }
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
    int rate = (config_.sampleRate == 8000 || config_.sampleRate == 24000)
                   ? config_.sampleRate
                   : 24000;
    std::ostringstream oss;
    oss << DEEPGRAM_TTS_BASE
        << "?model=" << config_.ttsVoice
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
