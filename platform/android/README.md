# Android Platform Layer

Uses **FusedLocationProvider** (GPS) and **SensorManager** (IMU, magnetometer). Raw sensor data via JNI. No business logic.

## Setup

1. Add to `build.gradle`:
   - `com.google.android.gms:play-services-location`
   - `com.android.billingclient:billing-ktx`
   - CMake for native `SensorBridge.cpp` + `EngineJNI.cpp`

2. Native lib `libfica_native.so` exports JNI symbols.

## Integration

```kotlin
val engineBridge = EngineBridge()
val sensorBridge = SensorBridge(context)

sensorBridge.setSink(engineBridge.getSensorSinkPtr())
sensorBridge.start()

engineBridge.setOrigin(-23.55, -46.63, 800.0)
engineBridge.setSubscriptionActive(billingIsActive)

// 100Hz: Choreographer or Handler
val state = engineBridge.tick(SystemClock.elapsedRealtimeNanos() / 1e9)
// state[0..2]=position, state[3..5]=velocity, state[6..9]=quat
```

## Data Flow

- **FusedLocationProvider** → `onLocationResult` → `nativePushGps`
- **SensorManager TYPE_ACCELEROMETER** → m/s² → `nativePushImu`
- **SensorManager TYPE_GYROSCOPE** → rad/s → `nativePushImu`
- **SensorManager TYPE_MAGNETIC_FIELD** → µT → `nativePushMag`

Engine expects: accel (m/s²), gyro (rad/s), mag (µT).
