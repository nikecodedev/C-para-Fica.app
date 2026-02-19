#pragma once

#include <string>
#include <vector>
#include <functional>
#include <cstdint>

namespace engine {
namespace audio {

/**
 * Platform-agnostic WebSocket transport for streaming API.
 * Platform implements using URLSessionWebSocketTask (iOS), OkHttp WebSocket (Android).
 */
class IWebSocketTransport {
public:
    virtual ~IWebSocketTransport() = default;

    /** Text message received (JSON from Deepgram). */
    using OnText = std::function<void(const std::string& text)>;

    /** Binary message received. */
    using OnBinary = std::function<void(const std::vector<std::uint8_t>& data)>;

    /** Connection opened. */
    using OnOpen = std::function<void()>;

    /** Connection closed or error. */
    using OnClose = std::function<void(const std::string& reason)>;

    /** Start connection. Calls onOpen when ready, onClose on error/close. */
    virtual void connect(const std::string& url,
                         const std::vector<std::pair<std::string, std::string>>& headers) = 0;

    /** Send binary frame (audio chunks). */
    virtual void sendBinary(const std::vector<std::uint8_t>& data) = 0;

    /** Send text frame (e.g. KeepAlive JSON). */
    virtual void sendText(const std::string& text) = 0;

    /** Close connection gracefully. */
    virtual void close() = 0;

    /** Check if connected. */
    virtual bool isConnected() const = 0;

    /** Set handlers. Must be set before connect. */
    virtual void setOnText(OnText cb) = 0;
    virtual void setOnBinary(OnBinary cb) = 0;
    virtual void setOnOpen(OnOpen cb) = 0;
    virtual void setOnClose(OnClose cb) = 0;
};

}  // namespace audio
}  // namespace engine
