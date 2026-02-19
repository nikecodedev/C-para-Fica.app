# Build Strategy — First Build

**Goal:** Running pipeline with engine, EKF, logging, 100Hz loop, and basic UI. No full tuning.

## First Build Checklist

### 1. Engine Skeleton ✓
- [ ] `EngineAccessManager` + `TrackingEngine` linked
- [ ] Origin set (from first GPS or config)
- [ ] Sensors feeding: `pushImu`, `pushGps`, `pushMag`

### 2. EKF Initialized ✓
- [ ] `ExtendedKalmanFilter` created with default config
- [ ] Initial state: position (0,0,0), identity quaternion, zero biases
- [ ] No parameter tuning—use defaults

### 3. Logging Active ✓
- [ ] `TrackingLogger` attached to engine via `setLogger`
- [ ] `exportToCsv` available for debugging (call on session end)
- [ ] No logging in hot path if it blocks

### 4. 100Hz Loop Running ✓
- [ ] Timer/CADisplayLink/Choreographer fires every 10ms
- [ ] `engine.tick(timestamp)` called each tick
- [ ] Subscription active for 100Hz (or accept 1Hz GPS-only initially)

### 5. Basic UI ✓
- [ ] **Speed** — display from `state.vx, vy, vz` (magnitude), use `UnitConverter`
- [ ] **Distance** — accumulated from position delta, use `UnitConverter`
- [ ] **Trajectory line** — append `(state.px, state.py)` to path, draw polyline

### 6. Explicitly Deferred
- [ ] EKF noise tuning (Q, R matrices)
- [ ] Magnetometer calibration
- [ ] Advanced trajectory visualization
- [ ] Full offline analysis

## Pipeline Diagram

```
[Sensors] → pushGps/pushImu/pushMag → [Buffers]
                                            ↓
[100Hz Timer] → tick(t) → [EKF predict + update] → TrackingState
                                            ↓
[UI] ← speed, distance, trajectory ← [Display layer]
```

## Quick Validation

1. Run app with sensors enabled
2. Walk/drive 50m
3. Check: speed non-zero when moving, distance ~50m, trajectory line visible
4. Export CSV, inspect in spreadsheet
5. If wildly wrong: check sensor units and origin; do NOT tune yet

## Files

| Component | Path |
|-----------|------|
| Integration | `app/ui/FirstBuildIntegration.hpp/.cpp` |
| iOS UI | `platform/ios/TrackViewController.h/.mm` |
| Android UI | `platform/android/TrackScreen.kt`, `res/layout/track_screen.xml` |
