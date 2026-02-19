# Audio Config

## API Key (Secure Storage)

API key must be provided by the platform layer. Set `AudioConfig::apiKey` from:

- Keychain (iOS) / Keystore (Android)
- Environment variable
- Secure config loaded at runtime

Example (pseudo):
```cpp
config.apiKey = platform::getSecureString("deepgram_api_key");
config.syncToDeepgram();  // optional: copy into deepgram overlay
engine.setConfig(config);
```

Never hardcode or log the API key. Use `DeepgramConfig::clearApiKey()` when done.

## DeepgramConfig (deepgram_config.hpp)

- **apiKey** — Set from platform secure storage
- **language** — Default "en" or "pt"
- **ttsVoice** — Default "aura-asteria-en"
- **ttsSampleRate** — 8000 or 24000
- **sttSampleRate** — e.g. 16000 for voice input
- **mode** — `RealTime` (low latency) or `Production` (quality)

RealTime: interim_results=true, utterance_end_ms=1000.  
Production: interim_results=false, utterance_end_ms=2000.
