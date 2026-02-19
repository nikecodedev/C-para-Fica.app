#pragma once

namespace engine {
namespace sensors {

/**
 * Pure virtual interface for all sensor types.
 * Platform layer injects RAW data; no smoothing is performed inside sensor classes.
 */
class ISensor {
public:
    virtual ~ISensor() = default;

    /**
     * Update sensor state at the given timestamp.
     * Called by the fusion engine each tick (50-100Hz).
     * Implementations pass through raw data only—no filtering or smoothing.
     */
    virtual void update(double timestamp) = 0;
};

}  // namespace sensors
}  // namespace engine
