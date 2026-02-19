# Localization (i18n)

Supports **Portuguese** and **English**. All UI strings use keys—never hardcode text.

## Usage

```cpp
#include "Localizer.hpp"
#include "LocalizationKeys.hpp"

app::localization::Localizer localizer;

// Auto-detect from system (platform passes locale string)
localizer.setLocaleFromSystem("pt-BR");  // → Portuguese

// Or set explicitly
localizer.setLocale("en");

// Lookup by key
std::string text = localizer.t(Keys::START_TRACKING);  // "Start Tracking" or "Iniciar Rastreamento"
```

## Platform Integration

**iOS (Swift):**
```swift
let locale = Locale.current.identifier  // e.g. "pt_BR"
localizer.setLocaleFromSystem(locale)
```

**Android (Kotlin):**
```kotlin
val locale = Locale.getDefault().toString()  // e.g. "pt_BR"
localizer.setLocaleFromSystem(locale)
```

**With LocaleDetector** (C++ pulls locale from platform):
- Platform implements `LocaleDetector::setProvider([]() { return getPlatformLocale(); });`
- C++ calls `localizer.setLocaleFromSystem(LocaleDetector::getSystemLocale());`

## Adding Strings

1. Add key to `LocalizationKeys.hpp`
2. Add translations to `Translations.cpp` (en and pt)
