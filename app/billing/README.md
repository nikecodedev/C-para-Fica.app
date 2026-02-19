# In-App Purchase

Subscription: **auto-renewable**, **7-day free trial**.

| Region | Price |
|--------|-------|
| US | $4.99 |
| Brazil | R$ 29.90 |

## Receipt Validation

Server-side validation ready:

- **iOS**: `SKReceiptRefreshRequest` → send receipt to server
- **Android**: `Purchase.getOriginalJson()` → verify with Google Play Developer API

## Integration

Platform implements `SubscriptionManager`. App wires status to engine:

```kotlin
// Android
billingManager.setOnStatusChanged { active ->
    engineBridge.setSubscriptionActive(active)
}
```

```objc
// iOS
manager.onStatusChanged = ^(BOOL active) {
    accessManager.setSubscriptionActive(active);
};
```

**Engine only receives boolean**—no billing logic in engine.
