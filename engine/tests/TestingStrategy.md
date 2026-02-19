# Testing Strategy

**Never test fusion blindly in field.** Validate math, replay, and simulate first.

## 1. Unit Tests (EKF Math)

- `test_ekf_math.cpp` — Quaternion, CoordinateConverter, Matrix
- Run before any integration
- Validates: identity, product, rotation, ECEF roundtrip, matrix invert

## 2. Replay Mode

- `SensorReplay` — Load CSV from `TrackingLogger.exportToCsv()`
- Feeds recorded raw sensors back into engine
- **Requirement:** Set engine origin before load (match recording session)
- Reproducible; no field required

```cpp
EngineAccessManager engine;
engine.getEngine().setOrigin(lat0, lon0, alt0);
SensorReplay replay(engine);
replay.load("session.csv");
TrackingState state;
while (replay.step(state)) { ... }
```

## 3. Simulation Mode

- `Simulator` — Synthetic trajectory and sensor data
- Set velocity, origin; step(dt) pushes fake GPS/IMU/mag
- Deterministic; use for fusion logic validation

```cpp
Simulator sim;
sim.setOrigin(-23.55, -46.63, 800);
sim.setTrajectory(5, 0, 0);  // 5 m/s east
for (int i = 0; i < 1000; ++i) {
    sim.step(0.01, engine.getEngine());
    engine.tick(sim.getTime());
}
```

## 4. Field Testing

Only after passing:
1. Unit tests
2. Replay from known-good sessions
3. Simulation with expected outputs
