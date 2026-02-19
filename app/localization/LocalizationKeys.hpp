#pragma once

#include <string>

namespace app {
namespace localization {

/**
 * All UI string keys. Never hardcode text—use these keys.
 * Keys are stable identifiers; translations live in language files.
 */
namespace Keys {

    // Navigation
    constexpr const char* HOME = "home";
    constexpr const char* SETTINGS = "settings";
    constexpr const char* TRACKING = "tracking";
    constexpr const char* SUBSCRIPTION = "subscription";

    // Tracking
    constexpr const char* START_TRACKING = "start_tracking";
    constexpr const char* STOP_TRACKING = "stop_tracking";
    constexpr const char* TRACKING_ACTIVE = "tracking_active";
    constexpr const char* TRACKING_PAUSED = "tracking_paused";
    constexpr const char* SPEED = "speed";
    constexpr const char* DISTANCE = "distance";
    constexpr const char* ELEVATION = "elevation";

    // Units
    constexpr const char* METRIC = "metric";
    constexpr const char* IMPERIAL = "imperial";

    // Subscription
    constexpr const char* SUBSCRIBE = "subscribe";
    constexpr const char* SUBSCRIBED = "subscribed";
    constexpr const char* UPGRADE_FOR_100HZ = "upgrade_for_100hz";
    constexpr const char* GPS_ONLY_MODE = "gps_only_mode";

    // Settings
    constexpr const char* LANGUAGE = "language";
    constexpr const char* UNITS = "units";
    constexpr const char* PORTUGUESE = "portuguese";
    constexpr const char* ENGLISH = "english";

    // Common
    constexpr const char* OK = "ok";
    constexpr const char* CANCEL = "cancel";
    constexpr const char* SAVE = "save";
    constexpr const char* ERROR = "error";
    constexpr const char* LOADING = "loading";

}  // namespace Keys

}  // namespace localization
}  // namespace app
