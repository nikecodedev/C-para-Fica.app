#pragma once

#include <string>

namespace app {
namespace license {

/**
 * Chave com Validade — time-limited license key for client tests and homologation.
 * When valid, the app treats it like an active subscription (full fusion enabled).
 *
 * Key format: "FICA-TEST-<unix_epoch>" where epoch is the validity end (seconds since 1970).
 * Example: "FICA-TEST-1738368000" valid until 2025-02-01 00:00:00 UTC.
 *
 * Platform sets the key (from server or config). Call isValid(currentTimeUnix) to gate access.
 */
class LicenseKeyWithValidity {
public:
    LicenseKeyWithValidity() = default;

    /** Set the license key (e.g. from server response or config). */
    void setKey(const std::string& key);

    /** Clear key and validity. */
    void clear();

    /**
     * True if key is set and current time is before validity end.
     * @param currentTimeUnix Current time in seconds since 1970 (platform provides).
     */
    bool isValid(double currentTimeUnix) const;

    /** Whether a key has been set (even if expired). */
    bool hasKey() const { return !key_.empty(); }

private:
    std::string key_;
    double validityEndUnix_{0.0};  // 0 = not set or no expiry in key
};

}  // namespace license
}  // namespace app
