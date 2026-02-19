#pragma once

#include "engine/audio/voice_agent/IVoiceAgent.hpp"
#include "engine/audio/tts/ITextToSpeech.hpp"
#include "ITrackingController.hpp"
#include "localization/Localizer.hpp"
#include "VoiceCommandQueue.hpp"
#include <memory>
#include <string>
#include <functional>

namespace app {
namespace audio {

/**
 * Integrates voice commands with tracking engine.
 * - "start_tracking" / "start tracking" → controller.startTracking()
 * - "stop_tracking" / "stop tracking" → controller.stopTracking()
 * - TTS feedback: speak speed, distance, trajectory updates
 *
 * Performance: Voice commands are queued and processed asynchronously.
 * Call drainVoiceCommands() from the main/tick thread to avoid blocking the 100Hz loop.
 */
class AudioTrackingIntegration {
public:
    AudioTrackingIntegration();

    /** Wire voice agent. Call after engine has VoiceAgent set. */
    void setVoiceAgent(engine::audio::IVoiceAgent* agent);

    /** Wire TTS for feedback. Optional. */
    void setTTS(engine::audio::ITextToSpeech* tts);

    /** Set controller for start/stop. Platform provides implementation. */
    void setTrackingController(ITrackingController* ctrl) { controller_ = ctrl; }

    /** Or use raw callbacks instead of controller. */
    void setOnStartTracking(std::function<void()> cb) { onStart_ = std::move(cb); }
    void setOnStopTracking(std::function<void()> cb) { onStop_ = std::move(cb); }

    /**
     * Speak current tracking state (speed, distance).
     * Call from platform when tracking active, e.g. every 5–10 seconds.
     */
    void speakFeedback(double speedDisplay, double distanceDisplay,
                       const char* speedUnit, const char* distanceUnit);

    /**
     * Speak feedback with localized labels from Localizer.
     * Uses Keys::SPEED and Keys::DISTANCE for PT/EN.
     */
    void speakFeedbackLocalized(const app::localization::Localizer& localizer,
                                double speedDisplay, double distanceDisplay,
                                const char* speedUnit, const char* distanceUnit);

    /** Normalize command for matching (trim, lowercase, collapse spaces). */
    static std::string normalizeCommand(const std::string& raw);

    /** Check if command matches start/stop. */
    static bool isStartTracking(const std::string& cmd);
    static bool isStopTracking(const std::string& cmd);

    /**
     * Enable async processing: voice callback enqueues; no work on WebSocket thread.
     * Call drainVoiceCommands() from main/tick thread to process.
     */
    void setUseAsyncCommands(bool use) { useAsyncCommands_ = use; }
    bool getUseAsyncCommands() const { return useAsyncCommands_; }

    /**
     * Drain queued voice commands. Call from 100Hz tick or main loop.
     * Non-blocking; processes commands without blocking the loop.
     */
    void drainVoiceCommands();

    /** Access queue for size/debug. */
    VoiceCommandQueue& getCommandQueue() { return commandQueue_; }

private:
    void onVoiceCommand(const std::string& cmd);

    void executeCommand(const std::string& cmd);

    ITrackingController* controller_{nullptr};
    std::function<void()> onStart_;
    std::function<void()> onStop_;
    engine::audio::IVoiceAgent* voiceAgent_{nullptr};
    engine::audio::ITextToSpeech* tts_{nullptr};
    bool useAsyncCommands_{true};
    VoiceCommandQueue commandQueue_;
};

}  // namespace audio
}  // namespace app
