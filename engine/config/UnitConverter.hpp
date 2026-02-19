#pragma once

namespace engine {
namespace config {

/** Unit system for display. Engine internally uses SI (m, m/s). */
enum class UnitSystem {
    Metric,   ///< km, m/s
    Imperial  ///< miles, mph
};

/**
 * Convert engine SI values to display units.
 * Engine always uses: distance (m), speed (m/s).
 */
class UnitConverter {
public:
    static constexpr double M_PER_KM = 1000.0;
    static constexpr double M_PER_MILE = 1609.344;
    static constexpr double MS_TO_KMH = 3.6;
    static constexpr double MS_TO_MPH = 2.23693629;

    UnitConverter() : system_(UnitSystem::Metric) {}
    explicit UnitConverter(UnitSystem system) : system_(system) {}

    void setUnitSystem(UnitSystem system) { system_ = system; }
    UnitSystem getUnitSystem() const { return system_; }

    /** SI distance (m) → display value */
    double distanceToDisplay(double meters) const;

    /** SI speed (m/s) → display value */
    double speedToDisplay(double m_per_sec) const;

    /** Display → SI distance (m) */
    double displayToDistance(double display) const;

    /** Display → SI speed (m/s) */
    double displayToSpeed(double display) const;

    /** Unit label for distance */
    const char* distanceUnitLabel() const;

    /** Unit label for speed */
    const char* speedUnitLabel() const;

    /**
     * Set unit system from region. Metric: most countries. Imperial: US, UK, etc.
     * Pass region code (e.g. "US", "GB" → Imperial; "BR", "DE" → Metric).
     */
    void setFromRegion(const char* regionCode);

private:
    UnitSystem system_;
};

}  // namespace config
}  // namespace engine
