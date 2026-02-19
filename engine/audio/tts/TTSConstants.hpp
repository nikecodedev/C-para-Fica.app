#pragma once

#include <string>

namespace engine {
namespace audio {

/** Language codes for TTS. PT/EN support. */
namespace Lang {
    constexpr const char* EN = "en";
    constexpr const char* PT = "pt";
}

/** Deepgram Aura voice IDs for selection. */
namespace Voice {
    constexpr const char* ASTERIA_EN = "aura-asteria-en";
    constexpr const char* LUNA_EN   = "aura-luna-en";
    constexpr const char* STELLA_EN = "aura-stella-en";
    constexpr const char* LUNA_PT   = "aura-luna-pt";
    constexpr const char* ASTERIA_PT = "aura-asteria-pt";
}

}  // namespace audio
}  // namespace engine
