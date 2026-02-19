# Audio–Tracking Integration

Connects voice commands to the tracking engine and TTS feedback.

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
```
