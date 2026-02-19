#pragma once

#include <functional>

namespace app {
namespace billing {

/**
 * IAP abstraction. Platform implements with StoreKit (iOS) / Play Billing (Android).
 * Engine only receives boolean—no billing logic in engine.
 */
class SubscriptionManager {
public:
    virtual ~SubscriptionManager() = default;

    /** Current subscription status. Engine calls setSubscriptionActive(result). */
    virtual bool isSubscriptionActive() const = 0;

    /** Initiate purchase flow */
    virtual void purchase() = 0;

    /** Restore purchases */
    virtual void restore() = 0;

    /** Called when status changes. Platform invokes; app wires to engine. */
    using StatusCallback = std::function<void(bool active)>;
    virtual void setStatusCallback(StatusCallback cb) = 0;
};

}  // namespace billing
}  // namespace app
