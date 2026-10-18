#pragma once

#include "altitude_display_source_policy.hpp"

#include <cmath>
#include <cstdint>
#include <string>

namespace bluesky::operations {

inline constexpr double kStandardSeaLevelPressureHpa = 1013.25;

// Timestamp values passed to this boundary must already be normalized to the
// same monotonic clock domain. MAVLink boot-time counters must not be compared
// directly with host wall-clock timestamps.
struct AltitudeSensorProvenance {
    std::string source_id;
    std::int64_t source_timestamp_ms{0};
    std::int64_t received_timestamp_ms{0};
    bool valid{false};
    bool fresh{false};
};

struct RadioAltimeterInput {
    double height_agl_m{0.0};
    std::string sensor_id;
    bool configured_as_radio_altimeter{false};
    bool downward_facing{false};
    AltitudeSensorProvenance provenance;
};

struct BarometricPressureInput {
    double static_pressure_hpa{0.0};
    std::string sensor_id;
    AltitudeSensorProvenance provenance;
};

inline bool altitude_provenance_usable(const AltitudeSensorProvenance& p) {
    return p.valid && p.fresh && !p.source_id.empty() &&
           p.source_timestamp_ms > 0 && p.received_timestamp_ms > 0 &&
           p.source_timestamp_ms <= p.received_timestamp_ms;
}

// A generic downward rangefinder is not automatically a radio altimeter.
// Capability/configuration must explicitly identify the installed sensor.
inline AltitudeReading make_radio_altimeter_reading(
    const RadioAltimeterInput& input) {
    if (!input.configured_as_radio_altimeter ||
        !input.downward_facing ||
        input.sensor_id.empty() ||
        !altitude_provenance_usable(input.provenance) ||
        !std::isfinite(input.height_agl_m) ||
        input.height_agl_m < 0.0) {
        return {};
    }

    return {input.height_agl_m, true, true,
            input.provenance.source_id,
            input.provenance.source_timestamp_ms,
            input.provenance.received_timestamp_ms};
}

// Convert measured absolute static pressure to ISA pressure altitude using
// the standard sea-level pressure reference 1013.25 hPa. This is pressure
// altitude, not height AGL and not terrain elevation. The adapter must supply
// absolute static pressure in hPa, not differential/dynamic pressure.
inline AltitudeReading make_standard_pressure_altitude_reading(
    const BarometricPressureInput& input) {
    if (input.sensor_id.empty() ||
        !altitude_provenance_usable(input.provenance) ||
        !std::isfinite(input.static_pressure_hpa) ||
        input.static_pressure_hpa < 100.0 ||
        input.static_pressure_hpa > 1200.0) {
        return {};
    }

    constexpr double exponent = 0.1902632365;
    constexpr double metres_per_pressure_factor = 44330.0;
    const double pressure_altitude_m = metres_per_pressure_factor *
        (1.0 - std::pow(input.static_pressure_hpa /
                            kStandardSeaLevelPressureHpa,
                        exponent));

    if (!std::isfinite(pressure_altitude_m)) {
        return {};
    }

    return {pressure_altitude_m, true, true,
            input.provenance.source_id,
            input.provenance.source_timestamp_ms,
            input.provenance.received_timestamp_ms};
}

} // namespace bluesky::operations
