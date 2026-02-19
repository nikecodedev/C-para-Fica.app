#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

/**
 * iOS audio bridge. Provides TTS playback (AVAudioEngine) and mic capture (AVAudioEngine input).
 * Creates C++ AudioEngine with Deepgram transports. Platform-agnostic engine lives in C++.
 */
@interface IOSAudioBridge : NSObject

/** Initialize. Call setApiKey before using TTS/VoiceAgent. */
- (instancetype)init;

/** Set Deepgram API key (from Keychain/secure storage). */
- (void)setApiKey:(NSString *)apiKey;

/** Get C++ AudioEngine pointer for integration. Engine owns TTS + VoiceAgent. */
- (void *)getAudioEnginePtr;

/** Speak text via TTS. */
- (void)speak:(NSString *)text;

/** Start voice agent listening. Commands delivered via onCommand callback. */
- (void)startListening;

/** Stop voice agent. */
- (void)stopListening;

/** Set block called when voice command is transcribed. */
- (void)setOnCommand:(void (^)(NSString *command))block;

- (void)dispose;

@end

NS_ASSUME_NONNULL_END
