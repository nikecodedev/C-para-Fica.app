# Audio Engine

Platform-agnostic voice/audio module for Deepgram integration. No iOS/Android SDK calls inside `/engine/audio`.

## Structure

| Path | Purpose |
|------|---------|
| `tts/` | Text-to-Speech interface |
| `voice_agent/` | Voice Agent (real-time commands) interface |
| `config/` | Configuration (API key from platform) |
| `logging/` | Logging interface |
| `tests/` | Unit tests |

## Interfaces

- **ITextToSpeech** — `synthesize(text, callback)` → PCM audio
- **IVoiceAgent** — `startListening(onTranscript, onError)` → transcribed text

## API Key

Never hardcode. Platform layer loads from:
- Keychain (iOS) / Keystore (Android)
- Environment variable `DEEPGRAM_API_KEY`
- Secure config file

## Platform Implementation

Platform (`platform/ios`, `platform/android`) implements:
- HTTP client for TTS (Deepgram REST)
- WebSocket + audio capture for Voice Agent
- Pass API key from secure storage to `AudioConfig`
