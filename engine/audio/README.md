# Audio Engine

Platform-agnostic voice/audio module for Deepgram integration. No iOS/Android SDK calls inside `/engine/audio`.

## Structure

| Path | Purpose |
|------|---------|
| `tts/` | Text-to-Speech interface + DeepgramTTS implementation |
| `voice_agent/` | Voice Agent interface + DeepgramVoiceAgent implementation |
| `transport/` | HTTP, WebSocket, Audio, Timer interfaces (platform implements) |
| `config/` | Configuration (API key from platform) |
| `logging/` | Logging interface |
| `tests/` | Unit tests |

## Interfaces

- **ITextToSpeech** — `speak(text)`, `setLanguage`, `setVoice`
- **IVoiceAgent** — `startListening`, `stopListening`, `setOnCommand(callback)`

## Deepgram Integration (Step 3)

### TTS (REST)
- **DeepgramTTS** implements ITextToSpeech
- Uses `IHttpTransport` to POST to `https://api.deepgram.com/v1/speak`
- Request: `{"text":"..."}`, query `model`, `encoding=linear16`, `sample_rate`
- Response: raw linear16 PCM → played via `IAudioSink`
- Low-latency: linear16 for immediate streaming playback

### Voice Agent (WebSocket)
- **DeepgramVoiceAgent** implements IVoiceAgent
- Uses `IWebSocketTransport` to connect to `wss://api.deepgram.com/v1/listen`
- Streams live mic audio (linear16) via `IAudioSource` → `sendBinary`
- Receives JSON transcripts → parses `transcript`, `speech_final`
- On final utterance → invokes `onCommand(transcript)`
- KeepAlive every 4s during silence (via `ITimer`) to prevent timeouts

### Platform Responsibilities
Platform must provide implementations of:
- **IHttpTransport** — NSURLSession (iOS), OkHttp (Android)
- **IWebSocketTransport** — URLSessionWebSocketTask (iOS), OkHttp WebSocket (Android)
- **IAudioSink** — AVAudioEngine (iOS), AudioTrack (Android)
- **IAudioSource** — AVAudioEngine input (iOS), AudioRecord (Android)
- **ITimer** — GCD / Handler for KeepAlive

### Example Setup
```cpp
auto http = platform::createHttpTransport();
auto ws = platform::createWebSocketTransport();
auto sink = platform::createAudioSink(24000);
auto source = platform::createAudioSource(16000);
auto timer = platform::createTimer();

AudioConfig config;
config.apiKey = platform::getApiKey();

engine.setTTS(std::make_unique<DeepgramTTS>(http, sink, config));
engine.setVoiceAgent(std::make_unique<DeepgramVoiceAgent>(ws, source, timer, config));
```

## API Key

Never hardcode. Platform layer loads from:
- Keychain (iOS) / Keystore (Android)
- Environment variable `DEEPGRAM_API_KEY`
- Secure config file
