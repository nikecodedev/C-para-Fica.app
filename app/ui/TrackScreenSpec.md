# Basic Track Screen — First Build UI

**Components:** Speed, Distance, Trajectory line. Minimal, functional.

## Layout

```
┌─────────────────────────────────┐
│  FICA MOTOR          [■] Stop   │
├─────────────────────────────────┤
│                                 │
│     SPEED                       │
│     12.3 km/h                   │
│                                 │
│     DISTANCE                    │
│     1.24 km                     │
│                                 │
├─────────────────────────────────┤
│  ┌─────────────────────────┐   │
│  │                         │   │
│  │    [Trajectory map]     │   │
│  │    polyline overlay     │   │
│  │                         │   │
│  └─────────────────────────┘   │
│                                 │
│  [▶] Start Tracking             │
└─────────────────────────────────┘
```

## Data Binding

| UI Element   | Source                         | Conversion          |
|--------------|--------------------------------|---------------------|
| Speed value  | `sqrt(vx²+vy²+vz²)` from state | `UnitConverter.speedToDisplay` |
| Distance     | Accumulated `Σ ‖Δp‖`          | `UnitConverter.distanceToDisplay` |
| Trajectory   | List of `(px, py)` in ENU      | Scale to viewport   |

## Update Rate

- Refresh UI at 10–20 Hz (not 100Hz) to avoid jank
- Throttle: use every 5th state or 50ms timer
