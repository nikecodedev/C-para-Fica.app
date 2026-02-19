# Audio–Tracking Integration

Connects voice commands to the tracking engine, TTS feedback, and i18n.

## Performance (100Hz Loop)

Voice commands are **queued** and processed asynchronously. Call `drainVoiceCommands()` from the same thread that runs the 100Hz tick:

```cpp
firstBuild.setAudioIntegration(&integration);
// In 100Hz timer:
firstBuild.tick(timestamp);  // drains queue, then engine tick
```

`setUseAsyncCommands(true)` (default): voice callback enqueues only; no work on WebSocket thread.

## Voice Commands

| Command | Action |
|---------|--------|
| "start tracking" / "start_tracking" | `controller.startTracking()` |
| "stop tracking" / "stop_tracking" | `controller.stopTracking()` |

Platform implements `ITrackingController` (or uses `CallbackTrackingController`):
- **startTracking** → `SensorBridge.start()` + `setSubscriptionActive(true)`
- **stopTracking** → `SensorBridge.stop()` + `setSubscriptionActive(false)`

## TTS Feedback

`speakFeedback(speedDisplay, distanceDisplay, speedUnit, distanceUnit)` speaks current state, e.g.:
> "Speed 12.3 km/h. Distance 1.24 km"

Call periodically (e.g. every 5–10 seconds) when tracking is active.

## Platform Wiring

```cpp
// Create integration
AudioTrackingIntegration integration;

// Option A: Callbacks (e.g. from JNI)
auto ctrl = std::make_unique<CallbackTrackingController>();
ctrl->setOnStart([&sensorBridge, &engineBridge]() {
    sensorBridge.start();
    engineBridge.setSubscriptionActive(true);
});
ctrl->setOnStop([&sensorBridge, &engineBridge]() {
    sensorBridge.stop();
    engineBridge.setSubscriptionActive(false);
});
integration.setTrackingController(ctrl.get());

// Wire audio engine
integration.setVoiceAgent(audioEngine.getVoiceAgent());
integration.setTTS(audioEngine.getTTS());

// Periodic TTS feedback (when tracking active)
integration.speakFeedback(ui.getSpeedDisplay(), ui.getDistanceDisplay(),
                         ui.getSpeedUnit(), ui.getDistanceUnit());

// Localized feedback (PT/EN from Localizer)
integration.speakFeedbackLocalized(localizer, speed, distance, speedUnit, distUnit);
```

## Localization (PT/EN)

Use **TTSLocalizationSync** to keep TTS in sync with the app locale:

```cpp
// On startup or when locale changes
localizer.setLocaleFromSystem(LocaleDetector::getSystemLocale());
TTSLocalizationSync::sync(localizer, audioEngine.getTTS());

// Or directly from system locale
TTSLocalizationSync::syncFromSystem("pt-BR", audioEngine.getTTS());
```

Also update engine config for Voice Agent (STT language):
```cpp
config.language = localizer.getLocale();
config.deepgram.language = config.language;
audioEngine.setConfig(config);
```
