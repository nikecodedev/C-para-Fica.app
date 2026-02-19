#include "AudioTrackingIntegration.hpp"
#include <algorithm>
#include <cctype>
#include <sstream>

namespace app {
namespace audio {

AudioTrackingIntegration::AudioTrackingIntegration() = default;

void AudioTrackingIntegration::setVoiceAgent(engine::audio::IVoiceAgent* agent) {
    voiceAgent_ = agent;
    if (agent) {
        agent->setOnCommand([this](const std::string& cmd) { onVoiceCommand(cmd); });
    }
}

void AudioTrackingIntegration::setTTS(engine::audio::ITextToSpeech* tts) {
    tts_ = tts;
}

void AudioTrackingIntegration::speakFeedback(double speedDisplay, double distanceDisplay,
                                             const char* speedUnit, const char* distanceUnit) {
    if (!tts_) return;

    std::ostringstream oss;
    oss.precision(1);
    oss << std::fixed;
    oss << "Speed " << speedDisplay << " " << (speedUnit ? speedUnit : "km/h");
    oss << ". Distance " << distanceDisplay << " " << (distanceUnit ? distanceUnit : "km");
    tts_->speak(oss.str());
}

std::string AudioTrackingIntegration::normalizeCommand(const std::string& raw) {
    std::string out;
    out.reserve(raw.size());
    bool prevSpace = false;
    for (char c : raw) {
        if (std::isspace(static_cast<unsigned char>(c)) || c == '_') {
            if (!prevSpace) { out += ' '; prevSpace = true; }
        } else {
            out += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
            prevSpace = false;
        }
    }
    while (!out.empty() && out.front() == ' ') out.erase(0, 1);
    while (!out.empty() && out.back() == ' ') out.pop_back();
    return out;
}

bool AudioTrackingIntegration::isStartTracking(const std::string& cmd) {
    std::string n = normalizeCommand(cmd);
    return n == "start tracking" || n == "start_tracking" || n == "start track";
}

bool AudioTrackingIntegration::isStopTracking(const std::string& cmd) {
    std::string n = normalizeCommand(cmd);
    return n == "stop tracking" || n == "stop_tracking" || n == "stop track";
}

void AudioTrackingIntegration::onVoiceCommand(const std::string& cmd) {
    if (isStartTracking(cmd)) {
        if (controller_) controller_->startTracking();
        else if (onStart_) onStart_();
    } else if (isStopTracking(cmd)) {
        if (controller_) controller_->stopTracking();
        else if (onStop_) onStop_();
    }
}

}  // namespace audio
}  // namespace app
