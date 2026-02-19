# Audio Tests

## Unit tests (test_deepgram.cpp)

### TTS playback
- **Basic** — speak → HTTP POST → play PCM
- **Empty text** — no request sent
- **API failure** — no playback on error
- **URL format** — DeepgramConfig overrides

### Voice command transcription
- **Final transcript** — `speech_final: true` → callback
- **is_final** — alternate JSON field
- **Interim** — `speech_final: false` → no callback
- **Transcript with spaces** — `"transcript": "text"` format

### Replay recorded audio
- **FileAudioSource** — reads raw PCM (linear16, mono) from file
- **Replay flow** — file → chunks → Voice Agent → WebSocket binary send
- **Fixture** — `writeTestPcmFile()` creates a small raw PCM file at runtime

Run: `ctest -R DeepgramTest` or `./deepgram_test` (from build dir).
