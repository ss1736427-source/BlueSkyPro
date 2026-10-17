#pragma once

#include "mavlink_altitude_display_adapter.hpp"
#include "mavlink_telemetry_decoder_boundary.hpp"

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <tuple>

namespace bluesky::operations {

// Per-vehicle/source/component cache. It accepts only already-decoded,
// session-admitted messages. The cache is display-only; it cannot establish
// mission readiness or flight safety.
class MavlinkAltitudeDisplayRuntime final {
public:
    explicit MavlinkAltitudeDisplayRuntime(
        MavlinkAltitudeDisplaySensorConfig sensor_config,
        std::string vehicle_id, std::string source_id,
        std::uint8_t system_id, std::uint8_t component_id);

    // Returns true only when the accepted telemetry advanced the cached state.
    // A duplicate or out-of-order source update never replaces newer data.
    bool ingestAccepted(const DecodedMavlinkMessage& message);

    MavlinkAltitudeDisplayAdapterResult evaluate(std::int64_t now_ms) const;
    bool hasAnyAltitudeInput() const;

private:
    using SourceKey = std::tuple<std::string, std::string, std::uint8_t, std::uint8_t>;

    struct CachedDistance {
        MavlinkDistanceSensorSample sample;
        bool timestamp_quality_degraded{false};
    };

    struct CachedPressure {
        MavlinkBarometricPressureSample sample;
        bool timestamp_quality_degraded{false};
    };

    static SourceKey sourceKey(const DecodedMavlinkMessage& message);
    static bool advances(std::uint32_t incoming_boot_ms,
                         std::int64_t incoming_received_ms,
                         std::uint32_t cached_boot_ms,
                         std::int64_t cached_received_ms);

    MavlinkAltitudeDisplaySensorConfig sensor_config_;
    std::string vehicle_id_;
    std::string source_id_;
    std::uint8_t system_id_{0};
    std::uint8_t component_id_{0};
    std::map<SourceKey, std::map<std::uint8_t, CachedDistance>> distance_by_source_;
    std::map<SourceKey, CachedPressure> pressure_by_source_;
};

} // namespace bluesky::operations
