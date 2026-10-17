#pragma once

#include "altitude_source_telemetry_contract.hpp"
#include "mavlink_telemetry_mapping.hpp"

#include <cstdint>
#include <string>

namespace bluesky::operations {

inline constexpr std::uint8_t kMavSensorOrientationPitch270 = 25;

struct MavlinkAltitudeDisplaySensorConfig {
    bool radio_altimeter_configured{false};
    std::uint8_t radio_altimeter_sensor_id{0};
    std::uint8_t radio_altimeter_sensor_type{0};
    std::int64_t maximum_age_ms{1000};
    std::int64_t maximum_pair_skew_ms{500};
};

struct MavlinkAltitudeDisplayAdapterResult {
    AltitudeDisplayDecision decision;
    bool timestamp_quality_degraded{false};
    bool radio_altimeter_input_usable{false};
    bool barometric_input_usable{false};
};

inline bool altitude_sample_recent(std::int64_t received_timestamp_ms,
                                   std::int64_t now_ms,
                                   std::int64_t maximum_age_ms) {
    return received_timestamp_ms > 0 && now_ms > 0 &&
           maximum_age_ms >= 0 &&
           received_timestamp_ms <= now_ms &&
           (now_ms - received_timestamp_ms) <= maximum_age_ms;
}

// Connects decoded MAVLink source-specific telemetry to the 100 m display
// selector. The radio-altimeter sensor must be explicitly configured by
// vehicle capability and match both sensor ID and type; downward orientation
// alone is not sufficient to identify a radio altimeter.
inline MavlinkAltitudeDisplayAdapterResult select_mavlink_altitude_display(
    const MavlinkTelemetrySample& sample,
    const MavlinkAltitudeDisplaySensorConfig& config,
    std::int64_t now_ms) {
    AltitudeReading radio_reading;
    AltitudeReading barometric_reading;

    if (config.radio_altimeter_configured &&
        sample.distance_sensor.has_value()) {
        const auto& distance = *sample.distance_sensor;
        const bool matching_sensor =
            distance.sensor_id == config.radio_altimeter_sensor_id &&
            distance.sensor_type == config.radio_altimeter_sensor_type &&
            distance.orientation == kMavSensorOrientationPitch270 &&
            distance.valid &&
            distance.distance_m.has_value() &&
            distance.source_time_boot_ms > 0;
        const bool fresh = altitude_sample_recent(
            distance.received_timestamp_ms, now_ms, config.maximum_age_ms);
        if (matching_sensor) {
            RadioAltimeterInput input;
            input.height_agl_m = *distance.distance_m;
            input.sensor_id = std::to_string(distance.sensor_id);
            input.configured_as_radio_altimeter = true;
            input.downward_facing = true;
            input.provenance = {
                sample.source_id + ":DISTANCE_SENSOR:" +
                    std::to_string(distance.sensor_id),
                distance.received_timestamp_ms,
                distance.received_timestamp_ms,
                true,
                fresh
            };
            radio_reading = make_radio_altimeter_reading(input);
        }
    }

    if (sample.barometric_pressure.has_value()) {
        const auto& pressure = *sample.barometric_pressure;
        const bool fresh = pressure.valid &&
            pressure.source_time_boot_ms > 0 &&
            altitude_sample_recent(pressure.received_timestamp_ms,
                                   now_ms, config.maximum_age_ms);
        if (pressure.valid && pressure.source_time_boot_ms > 0) {
            BarometricPressureInput input;
            input.static_pressure_hpa = pressure.absolute_pressure_hpa;
            input.sensor_id = sample.source_id;
            input.provenance = {
                sample.source_id + ":SCALED_PRESSURE",
                pressure.received_timestamp_ms,
                pressure.received_timestamp_ms,
                true,
                fresh
            };
            barometric_reading =
                make_standard_pressure_altitude_reading(input);
        }
    }

    if (radio_reading.valid && barometric_reading.valid) {
        const auto radio_received = radio_reading.received_timestamp_ms;
        const auto baro_received = barometric_reading.received_timestamp_ms;
        const auto skew = radio_received >= baro_received
            ? radio_received - baro_received
            : baro_received - radio_received;
        if (config.maximum_pair_skew_ms < 0 ||
            skew > config.maximum_pair_skew_ms) {
            barometric_reading.fresh = false;
        }
    }

    MavlinkAltitudeDisplayAdapterResult result;
    result.radio_altimeter_input_usable =
        altitude_reading_usable(radio_reading, true);
    result.barometric_input_usable =
        altitude_reading_usable(barometric_reading, false);
    result.decision = select_altitude_display(radio_reading,
                                               barometric_reading);
    result.timestamp_quality_degraded =
        sample.timestamp_is_receive_time_fallback;
    return result;
}

} // namespace bluesky::operations
