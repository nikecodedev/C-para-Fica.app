# iOS Audio Bridge

Platform layer for Deepgram TTS and Voice Agent. Uses AVAudioEngine for TTS playback and mic capture.

## Components

| Component | Implementation |
|-----------|-----------------|
| **IHttpTransport** | NSURLSession POST |
| **IWebSocketTransport** | URLSessionWebSocketTask |
| **IAudioSink** | AVAudioEngine + AVAudioPlayerNode (linear16 PCM) |
| **IAudioSource** | AVAudioEngine inputNode installTap (16kHz mono) |
| **ITimer** | dispatch_source_t (KeepAlive) |

## Usage

```objc
#import "IOSAudioBridge.h"

IOSAudioBridge *audio = [[IOSAudioBridge alloc] init];
[audio setApiKey:@"your-deepgram-api-key"];  // From Keychain

[audio setOnCommand:^(NSString *command) {
    NSLog(@"Voice command: %@", command);
}];

[audio speak:@"Hello world"];
[audio startListening];
// ... later
[audio stopListening];
[audio dispose];
```

## Format

- **TTS**: linear16, 24kHz mono (Deepgram default)
- **Mic**: linear16, 16kHz mono (Deepgram streaming)

## Dependencies

- AVFoundation
- Foundation
