# Audio Config

API key must be provided by the platform layer. Set `AudioConfig::apiKey` from:

- Keychain (iOS) / Keystore (Android)
- Environment variable
- Secure config loaded at runtime

Example (pseudo):
```cpp
config.apiKey = platform::getSecureString("deepgram_api_key");
```
