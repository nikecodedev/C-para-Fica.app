#import "IOSAudioBridge.h"
#import <AVFoundation/AVFoundation.h>
#import <dispatch/dispatch.h>

#include "engine/audio/AudioEngine.hpp"
#include "engine/audio/tts/DeepgramTTS.hpp"
#include "engine/audio/voice_agent/DeepgramVoiceAgent.hpp"
#include "engine/audio/transport/IHttpTransport.hpp"
#include "engine/audio/transport/IWebSocketTransport.hpp"
#include "engine/audio/transport/IAudioSink.hpp"
#include "engine/audio/transport/IAudioSource.hpp"
#include "engine/audio/transport/ITimer.hpp"

#include <memory>
#include <string>
#include <vector>
#include <mutex>
#include <atomic>

// MARK: - iOS IHttpTransport (NSURLSession)

@interface IOSHttpSession : NSObject <NSURLSessionDataDelegate>
@property (nonatomic, strong) NSMutableData *responseData;
@property (nonatomic, copy) void (^completion)(BOOL success, int statusCode, NSData *body);
@end

@implementation IOSHttpSession
- (instancetype)init {
    if (self = [super init]) _responseData = [NSMutableData data];
    return self;
}
- (void)URLSession:(NSURLSession *)session dataTask:(NSURLSessionDataTask *)task didReceiveData:(NSData *)data {
    [_responseData appendData:data];
}
- (void)URLSession:(NSURLSession *)session task:(NSURLSessionTask *)task didCompleteWithError:(NSError *)error {
    NSHTTPURLResponse *resp = (NSHTTPURLResponse *)task.response;
    int code = resp ? (int)resp.statusCode : 0;
    BOOL ok = (error == nil && code >= 200 && code < 300);
    if (_completion) _completion(ok, code, _responseData ?: [NSData data]);
}
@end

class IOSHttpTransport : public engine::audio::IHttpTransport {
public:
    void post(const std::string& url, const std::vector<Header>& headers,
              const std::vector<std::uint8_t>& body, OnComplete onComplete) override {
        NSURL *nsurl = [NSURL URLWithString:[NSString stringWithUTF8String:url.c_str()]];
        if (!nsurl) { if (onComplete) onComplete(false, 0, {}); return; }
        NSMutableURLRequest *req = [NSMutableURLRequest requestWithURL:nsurl];
        req.HTTPMethod = @"POST";
        req.HTTPBody = [NSData dataWithBytes:body.data() length:body.size()];
        for (const auto& h : headers) {
            [req setValue:[NSString stringWithUTF8String:h.second.c_str()]
      forHTTPHeaderField:[NSString stringWithUTF8String:h.first.c_str()]];
        }
        IOSHttpSession *session = [[IOSHttpSession alloc] init];
        session.completion = ^(BOOL success, int statusCode, NSData *respBody) {
            std::vector<std::uint8_t> vec(respBody.length);
            if (respBody.length) [respBody getBytes:vec.data() length:respBody.length];
            if (onComplete) onComplete(success, statusCode, vec);
        };
        NSURLSession *s = [NSURLSession sessionWithConfiguration:[NSURLSessionConfiguration defaultSessionConfiguration]
                                                        delegate:session delegateQueue:nil];
        [[s dataTaskWithRequest:req] resume];
    }
};

// MARK: - iOS IWebSocketTransport (URLSessionWebSocketTask)

@interface IOSWebSocketDelegate : NSObject <NSURLSessionWebSocketDelegate>
@property (nonatomic, copy) void (^onOpen)(void);
@property (nonatomic, copy) void (^onClose)(NSString *reason);
@property (nonatomic, copy) void (^onText)(NSString *text);
@end

@implementation IOSWebSocketDelegate
- (void)URLSession:(NSURLSession *)session webSocketTask:(NSURLSessionWebSocketTask *)task didOpenWithProtocol:(NSString *)protocol {
    if (_onOpen) _onOpen();
}
- (void)URLSession:(NSURLSession *)session webSocketTask:(NSURLSessionWebSocketTask *)task didCloseWithCode:(NSURLSessionWebSocketCloseCode)code reason:(NSData *)reason {
    NSString *r = reason.length ? [[NSString alloc] initWithData:reason encoding:NSUTF8StringEncoding] : @"";
    if (_onClose) _onClose(r ?: @"closed");
}
- (void)URLSession:(NSURLSession *)session task:(NSURLSessionTask *)task didCompleteWithError:(NSError *)error {
    if (error && _onClose) _onClose(error.localizedDescription.UTF8String ?: @"error");
}
@end

class IOSWebSocketTransport : public engine::audio::IWebSocketTransport {
public:
    IOSWebSocketTransport() : connected_(false) {}
    void connect(const std::string& url, const std::vector<std::pair<std::string, std::string>>& headers) override {
        NSURL *nsurl = [NSURL URLWithString:[NSString stringWithUTF8String:url.c_str()]];
        if (!nsurl) { if (onClose_) onClose_("invalid url"); return; }
        NSMutableURLRequest *req = [NSMutableURLRequest requestWithURL:nsurl];
        for (const auto& h : headers)
            [req setValue:[NSString stringWithUTF8String:h.second.c_str()]
      forHTTPHeaderField:[NSString stringWithUTF8String:h.first.c_str()]];
        delegate_ = [[IOSWebSocketDelegate alloc] init];
        __block IOSWebSocketTransport *w = this;
        delegate_.onOpen = ^{
            if (w) { w->connected_ = true; if (w->onOpen_) w->onOpen_(); }
        };
        delegate_.onClose = ^(NSString *r) {
            if (w) {
                w->connected_ = false;
                if (w->onClose_) w->onClose_(r.UTF8String ?: "closed");
            }
        };
        delegate_.onText = ^(NSString *t) {
            if (w && w->onText_ && t.UTF8String)
                w->onText_(std::string(t.UTF8String));
        };
        session_ = [NSURLSession sessionWithConfiguration:[NSURLSessionConfiguration defaultSessionConfiguration]
                                                 delegate:delegate_ delegateQueue:nil];
        task_ = [session_ webSocketTaskWithRequest:req];
        [task_ resume];
        receiveLoop();
    }
    void sendBinary(const std::vector<std::uint8_t>& data) override {
        if (!task_ || !connected_) return;
        NSData *d = [NSData dataWithBytes:data.data() length:data.size()];
        [task_ sendMessage:[[NSURLSessionWebSocketMessage alloc] initWithData:d] completionHandler:^(NSError *e){}];
    }
    void sendText(const std::string& text) override {
        if (!task_ || !connected_) return;
        NSString *s = [NSString stringWithUTF8String:text.c_str()];
        [task_ sendMessage:[[NSURLSessionWebSocketMessage alloc] initWithString:s] completionHandler:^(NSError *e){}];
    }
    void close() override {
        [task_ cancelWithCloseCode:NSURLSessionWebSocketCloseCodeNormalClosure reason:nil];
        task_ = nil; connected_ = false;
    }
    bool isConnected() const override { return connected_; }
    void setOnText(OnText cb) override { onText_ = cb; }
    void setOnBinary(OnBinary cb) override { onBinary_ = cb; }
    void setOnOpen(OnOpen cb) override { onOpen_ = cb; }
    void setOnClose(OnClose cb) override { onClose_ = cb; }
private:
    void receiveLoop() {
        if (!task_) return;
        __block IOSWebSocketTransport *w = this;
        [task_ receiveMessageWithCompletionHandler:^(NSURLSessionWebSocketMessage *msg, NSError *err) {
            if (msg.string && w->onText_) {
                const char *c = msg.string.UTF8String;
                if (c) w->onText_(std::string(c));
            }
            if (msg.data && w->onBinary_) {
                std::vector<std::uint8_t> v(msg.data.length);
                [msg.data getBytes:v.data() length:v.size()];
                w->onBinary_(v);
            }
            if (!err && w->connected_) w->receiveLoop();
        }];
    }
    NSURLSession *session_;
    NSURLSessionWebSocketTask *task_;
    IOSWebSocketDelegate *delegate_;
    std::atomic<bool> connected_;
    OnText onText_; OnBinary onBinary_; OnOpen onOpen_; OnClose onClose_;
};

// MARK: - iOS IAudioSink (AVAudioEngine)

class IOSAudioSink : public engine::audio::IAudioSink {
public:
    IOSAudioSink(int sampleRate) : sampleRate_(sampleRate) {
        engine_ = [[AVAudioEngine alloc] init];
        playerNode_ = [[AVAudioPlayerNode alloc] init];
        [engine_ attachNode:playerNode_];
        AVAudioFormat *fmt = [[AVAudioFormat alloc] initWithCommonFormat:AVAudioPCMFormatInt16
                                                              sampleRate:sampleRate
                                                                channels:1
                                                             interleaved:YES];
        [engine_ connect:playerNode_ to:engine_.mainMixerNode format:fmt];
        [engine_ startAndReturnError:nil];
    }
    void play(const std::vector<std::uint8_t>& pcmData) override {
        if (pcmData.empty()) return;
        AVAudioFormat *fmt = [[AVAudioFormat alloc] initWithCommonFormat:AVAudioPCMFormatInt16
                                                              sampleRate:sampleRate_
                                                                channels:1
                                                             interleaved:YES];
        AVAudioFrameCount frameCount = (AVAudioFrameCount)(pcmData.size() / 2);
        AVAudioPCMBuffer *buf = [[AVAudioPCMBuffer alloc] initWithPCMFormat:fmt frameCapacity:frameCount];
        buf.frameLength = frameCount;
        memcpy(buf.audioBufferList->mBuffers[0].mData, pcmData.data(), pcmData.size());
        [playerNode_ scheduleBuffer:buf completionHandler:nil];
        [playerNode_ play];
    }
    void stop() override { [playerNode_ stop]; }
    int getSampleRate() const override { return sampleRate_; }
private:
    int sampleRate_;
    AVAudioEngine *engine_;
    AVAudioPlayerNode *playerNode_;
};

// MARK: - iOS IAudioSource (AVAudioEngine input)

class IOSAudioSource : public engine::audio::IAudioSource {
public:
    IOSAudioSource(int sampleRate) : sampleRate_(sampleRate) {}
    void startCapture(OnAudioData onData) override {
        onData_ = onData;
        engine_ = [[AVAudioEngine alloc] init];
        AVAudioInputNode *input = engine_.inputNode;
        AVAudioFormat *target = [[AVAudioFormat alloc] initWithCommonFormat:AVAudioPCMFormatInt16
                                                                sampleRate:sampleRate_
                                                                  channels:1
                                                               interleaved:YES];
        [input installTapOnBus:0 bufferSize:2048 format:target block:^(AVAudioPCMBuffer *buf, AVAudioTime *time) {
            if (!onData_) return;
            size_t byteCount = buf.frameLength * 2;
            std::vector<std::uint8_t> v(byteCount);
            memcpy(v.data(), buf.audioBufferList->mBuffers[0].mData, byteCount);
            onData_(v);
        }];
        [engine_ startAndReturnError:nil];
    }
    void stopCapture() override {
        [engine_.inputNode removeTapOnBus:0];
        [engine_ stop];
        onData_ = nullptr;
    }
    int getSampleRate() const override { return sampleRate_; }
private:
    int sampleRate_;
    AVAudioEngine *engine_;
    OnAudioData onData_;
};

// MARK: - iOS ITimer (dispatch)

class IOSTimer : public engine::audio::ITimer {
public:
    void start(std::uint32_t intervalMs, std::function<void()> callback) override {
        stop();
        callback_ = callback;
        intervalMs_ = intervalMs;
        __block IOSTimer *w = this;
        source_ = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, dispatch_get_main_queue());
        dispatch_source_set_timer(source_, DISPATCH_TIME_NOW, intervalMs * NSEC_PER_MSEC, 0);
        dispatch_source_set_event_handler(source_, ^{
            if (w->callback_) w->callback_();
        });
        dispatch_resume(source_);
    }
    void stop() override {
        if (source_) { dispatch_source_cancel(source_); source_ = nil; }
        callback_ = nullptr;
    }
private:
    dispatch_source_t source_;
    std::function<void()> callback_;
    std::uint32_t intervalMs_;
};

// MARK: - IOSAudioBridge impl

@implementation IOSAudioBridge {
    std::unique_ptr<engine::audio::AudioEngine> engine_;
    std::shared_ptr<engine::audio::IHttpTransport> http_;
    std::shared_ptr<engine::audio::IWebSocketTransport> ws_;
    std::shared_ptr<engine::audio::IAudioSink> sink_;
    std::shared_ptr<engine::audio::IAudioSource> source_;
    std::shared_ptr<engine::audio::ITimer> timer_;
    engine::audio::AudioConfig config_;
    void (^onCommandBlock_)(NSString *);
}

- (instancetype)init {
    if (self = [super init]) {
        engine_ = std::make_unique<engine::audio::AudioEngine>();
        http_ = std::make_shared<IOSHttpTransport>();
        ws_ = std::make_shared<IOSWebSocketTransport>();
        sink_ = std::make_shared<IOSAudioSink>(24000);
        source_ = std::make_shared<IOSAudioSource>(16000);
        timer_ = std::make_shared<IOSTimer>();
        config_.sampleRate = 16000;
        config_.ttsVoice = "aura-asteria-en";
        config_.language = "en";
        engine_->setConfig(config_);
        engine_->setTTS(std::make_unique<engine::audio::DeepgramTTS>(http_, sink_, config_));
        engine_->setVoiceAgent(std::make_unique<engine::audio::DeepgramVoiceAgent>(ws_, source_, timer_, config_));
    }
    return self;
}

- (void)setApiKey:(NSString *)apiKey {
    if (apiKey) config_.apiKey = apiKey.UTF8String;
    engine_->setConfig(config_);
    if (auto *tts = dynamic_cast<engine::audio::DeepgramTTS*>(engine_->getTTS()))
        tts->setConfig(config_);
}

- (void *)getAudioEnginePtr {
    return engine_.get();
}

- (void)speak:(NSString *)text {
    if (auto *tts = dynamic_cast<engine::audio::DeepgramTTS*>(engine_->getTTS()))
        tts->setConfig(config_);
    if (engine_->getTTS())
        engine_->getTTS()->speak(text.UTF8String ?: "");
}

- (void)startListening {
    auto *va = engine_->getVoiceAgent();
    if (va) {
        __weak typeof(self) wself = self;
        va->setOnCommand([wself](const std::string& cmd) {
            IOSAudioBridge *s = wself;
            if (!s || !s->onCommandBlock_) return;
            NSString *str = [NSString stringWithUTF8String:cmd.c_str()];
            void (^block)(NSString *) = s->onCommandBlock_;
            dispatch_async(dispatch_get_main_queue(), ^{ if (block) block(str); });
        });
    }
    if (va) va->startListening();
}

- (void)stopListening {
    if (engine_->getVoiceAgent()) engine_->getVoiceAgent()->stopListening();
}

- (void)setOnCommand:(void (^)(NSString *))block {
    onCommandBlock_ = block;
}

- (void)dispose {
    engine_.reset();
}

@end
