# iOS Platform Layer

Uses **CoreLocation** (GPS) and **CoreMotion** (IMU, magnetometer). Raw sensor data only. No business logic.

## Integration

1. Add to your Xcode project:
   - `IOSSensorBridge.h`
   - `IOSSensorBridge.mm`
   - `SensorDataSink.hpp`
   - `EngineSensorAdapter.hpp`
   - Link: CoreLocation.framework, CoreMotion.framework

2. In AppDelegate or ViewController:

```objc
#import "IOSSensorBridge.h"
#include "platform/ios/EngineSensorAdapter.hpp"
#include "engine/core/EngineAccessManager.hpp"

// Create engine and adapter
engine::core::EngineAccessManager accessManager;
platform::ios::EngineSensorAdapter adapter(accessManager);

// Create bridge and set sink
IOSSensorBridge *bridge = [[IOSSensorBridge alloc] init];
[bridge setSensorSink:&adapter];

// Request location permission first, then:
[bridge start];
```

3. Run 100Hz tick loop (e.g. CADisplayLink or timer):

```objc
// Every 10ms - your display link or timer callback
double t = CACurrentMediaTime();
engine::core::TrackingState state = accessManager.tick(t);
// Use state.px, state.py, state.pz (ENU meters)
// Use UnitConverter to convert to km/mph for display
```

## Data Flow

- **CoreLocation** → `didUpdateLocations` → `pushGps(lat, lon, alt, accuracy)`
- **CoreMotion accelerometer** → raw G's → m/s² → `pushImu`
- **CoreMotion gyroscope** → rad/s → `pushImu`
- **CoreMotion magnetometer** → µT → `pushMag`

Units: Accel (m/s²), Gyro (rad/s), Mag (µT) — engine expects SI.
