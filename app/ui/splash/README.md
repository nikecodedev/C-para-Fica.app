# Splash Screen

**Design:** Sleek, minimal, premium. Fast load (~800ms).

## iOS

- `LaunchScreen.storyboard` — Add to Xcode project, set as Launch Screen
- Replace logo label with `logo.png` asset for production
- Background: #0A0A0F

## Android

- Extend `SplashActivity`, override `nextActivityClass()`:

```kotlin
class AppSplashActivity : SplashActivity() {
    override fun nextActivityClass() = MainActivity::class.java
}
```

- Set as launcher in AndroidManifest
- Add `logo.png` to drawable for production
