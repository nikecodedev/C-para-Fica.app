# Audio Performance

## Non-Blocking 100Hz Loop

The tracking engine runs at 100Hz (10ms tick). Audio processing must not block this loop.

| Component | Thread / Behavior |
|-----------|-------------------|
| **tick()** | Main/timer thread. No audio I/O. |
| **TTS speak()** | Returns immediately. HTTP runs async on platform network thread. |
| **TTS callback** | Platform thread (e.g. URLSession). Calls `play()`; platform must buffer, not block. |
| **Voice Agent** | WebSocket receives on platform thread. Callback enqueues command; does not execute controller. |
| **drainVoiceCommands()** | Called at start of tick. Processes queue; each command is quick (start/stop). |

## Async Voice Commands

Voice commands are queued in `VoiceCommandQueue` when received. The WebSocket callback does minimal work (push string) and returns.

`AudioTrackingIntegration::drainVoiceCommands()` is called from the **same thread as tick()**—typically at the start of each tick. This keeps controller actions (SensorBridge start/stop) on the correct thread and avoids blocking the network/WebSocket thread.

**Platform wiring:**
```cpp
firstBuild.setAudioIntegration(&integration);
// In timer callback (100Hz):
firstBuild.tick(timestamp);  // drains voice commands, then engine tick
```

## Buffering / Streaming

| Layer | Behavior |
|-------|----------|
| **Mic → WebSocket** | Platform captures in chunks (e.g. 2048 samples). Each chunk sent via `sendBinary()`; platform buffers WebSocket output. |
| **TTS → Play** | Deepgram returns linear16 PCM. Platform `IAudioSink::play()` receives full buffer; should schedule playback (e.g. `scheduleBuffer`) and return. |
| **Voice transcript** | Streaming: Deepgram sends JSON as speech is recognized. Final transcript triggers `onCommand`. |

Platform implementations must ensure:
- `IAudioSink::play()` returns immediately (queue buffer, don't block)
- `IWebSocketTransport::sendBinary()` buffers; doesn't block capture callback
- `IHttpTransport::post()` callback runs on completion; never on 100Hz thread
