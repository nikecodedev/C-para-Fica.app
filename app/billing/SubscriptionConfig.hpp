#pragma once

#include <string>

namespace app {
namespace billing {

/**
 * Subscription product configuration.
 * Platform uses these IDs with StoreKit / Play Billing.
 * Receipt validation structured for server-side validation readiness.
 */
namespace Config {

    /** iOS StoreKit product ID */
    constexpr const char* IOS_PRODUCT_ID = "com.ficamotor.tracking.premium";

    /** Android Play Billing product ID */
    constexpr const char* ANDROID_PRODUCT_ID = "tracking_premium";

    /** 7-day free trial, then auto-renewable */
    constexpr int TRIAL_DAYS = 7;

    /** Reference prices (for validation/docs; platform uses Store/Play pricing) */
    constexpr double PRICE_USD = 4.99;
    constexpr double PRICE_BRL = 29.90;

}

}  // namespace billing
}  // namespace app
