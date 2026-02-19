#pragma once

#include <string>
#include <vector>
#include <functional>
#include <cstdint>

namespace engine {
namespace audio {

/**
 * Platform-agnostic HTTP transport for REST API calls.
 * Platform implements using NSURLSession (iOS), OkHttp (Android), etc.
 */
class IHttpTransport {
public:
    virtual ~IHttpTransport() = default;

    /** HTTP header key-value pair. */
    using Header = std::pair<std::string, std::string>;

    /** Completion callback: (success, statusCode, body). */
    using OnComplete = std::function<void(bool success, int statusCode, const std::vector<std::uint8_t>& body)>;

    /**
     * Perform POST request.
     * @param url Full URL (e.g. https://api.deepgram.com/v1/speak?model=...)
     * @param headers Authorization, Content-Type, etc.
     * @param body Request body (JSON string or binary)
     * @param onComplete Called on completion (may be async)
     */
    virtual void post(const std::string& url,
                      const std::vector<Header>& headers,
                      const std::vector<std::uint8_t>& body,
                      OnComplete onComplete) = 0;
};

}  // namespace audio
}  // namespace engine
