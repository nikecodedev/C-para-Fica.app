# Audio Logging

## IAudioLogger

Platform-agnostic logging interface. Platform implements (console, file, remote).

## DeepgramLogger

Structured logging wrapper for Deepgram integration:

| Event | Format |
|-------|--------|
| **TTS request** | `[TTS] request: <preview> (len=N)` |
| **Voice command** | `[Voice] command: <text>` |
| **API response** | `[API] OK/FAIL status=N body=NB latency=Nms` |
| **Latency** | `[Latency] <op>: Nms` |
| **Stream** | `[Stream] opened/closed: <detail>` |

Platform sets logger via `AudioEngine::setLogger(IAudioLogger*)`. Forwarded to DeepgramTTS and DeepgramVoiceAgent.
