#pragma once

#include <cmath>
#include <cstdint>
#include <string>

namespace bluesky::operations {

inline constexpr double kRadioAltimeterDisplayLimitM = 100.0;

enum class AltitudeDisplaySource {
    Unavailable,
    RadioAltimeterAgl,
    BarometricStandardPressure,
    BarometricStandardPressureDegraded
};

// A source-specific reading. The adapter owns source decoding and freshness
// classification; this policy does not infer freshness from timestamps alone.
struct AltitudeReading {
    double value_m{0.0};
    bool valid{false};
    bool fresh{false};
    std::string source_id;
    std::int64_t source_timestamp_ms{0};
    std::int64_t received_timestamp_ms{0};
};

struct AltitudeDisplayDecision {
    AltitudeDisplaySource source{AltitudeDisplaySource::Unavailable};
    double value_m{0.0};
    bool value_available{false};
    bool degraded{false};
};

inline bool altitude_reading_usable(const AltitudeReading& reading,
                                    bool require_nonnegative = false) {
    return reading.valid &&
           reading.fresh &&
           !reading.source_id.empty() &&
           reading.source_timestamp_ms > 0 &&
           reading.received_timestamp_ms > 0 &&
           reading.source_timestamp_ms <= reading.received_timestamp_ms &&
           std::isfinite(reading.value_m) &&
           (!require_nonnegative || reading.value_m >= 0.0);
}

// Display policy only: it does not command flight controls or establish
// terrain clearance/flight readiness.
//
// <= 100 m measured AGL: radio altimeter.
// > 100 m measured AGL: barometric altitude referenced to standard pressure.
// If RA is unavailable, barometric altitude > 100 m may be shown only as a
// degraded fallback, never as an equivalent or verified AGL measurement.
inline AltitudeDisplayDecision select_altitude_display(
    const AltitudeReading& radio_altimeter_agl,
    const AltitudeReading& barometric_standard_pressure_altitude) {
    const bool radio_usable = altitude_reading_usable(radio_altimeter_agl, true);
    const bool baro_usable = altitude_reading_usable(
        barometric_standard_pressure_altitude, false);

    if (radio_usable &&
        radio_altimeter_agl.value_m <= kRadioAltimeterDisplayLimitM) {
        return {AltitudeDisplaySource::RadioAltimeterAgl,
                radio_altimeter_agl.value_m, true, false};
    }

    if (radio_usable &&
        radio_altimeter_agl.value_m > kRadioAltimeterDisplayLimitM) {
        if (baro_usable) {
            return {AltitudeDisplaySource::BarometricStandardPressure,
                    barometric_standard_pressure_altitude.value_m, true, false};
        }
        return {};
    }

    // No valid RA measurement means the system cannot prove the aircraft is
    // above the 100 m AGL crossover. Show BARO only as an explicitly degraded
    // indication, and only when its standard-pressure altitude exceeds 100 m.
    if (baro_usable &&
        barometric_standard_pressure_altitude.value_m >
            kRadioAltimeterDisplayLimitM) {
        return {AltitudeDisplaySource::BarometricStandardPressureDegraded,
                barometric_standard_pressure_altitude.value_m, true, true};
    }

    return {};
}

} // namespace bluesky::operations
