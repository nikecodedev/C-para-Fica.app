#pragma once

#include <string>

namespace engine {
namespace audio {

/** Language codes for TTS. EN, PT, ES, DE. */
namespace Lang {
    constexpr const char* EN = "en";
    constexpr const char* PT = "pt";
    constexpr const char* ES = "es";
    constexpr const char* DE = "de";
}

/** Deepgram Aura voice IDs. USA, UK, Spain, Portugal, Germany. */
namespace Voice {
    constexpr const char* ASTERIA_EN = "aura-asteria-en";
    constexpr const char* LUNA_EN   = "aura-luna-en";
    constexpr const char* LUNA_PT   = "aura-luna-pt";
    constexpr const char* ASTERIA_PT = "aura-asteria-pt";
    constexpr const char* NESTOR_ES = "aura-2-nestor-es";   // Peninsular (Spain)
    constexpr const char* VIKTORIA_DE = "aura-2-viktoria-de";  // Germany
}

}  // namespace audio
}  // namespace engine
