#pragma once

#include "normalized_telemetry_state.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace bluesky::operations {

struct MavlinkTelemetrySample {
    std::string vehicle_id;
    std::string source_id;
    std::string autopilot;
    std::string mavlink_version{"2"};
    std::uint8_t system_id{0};
    std::uint8_t component_id{0};

    std::int64_t source_timestamp_ms{0};
    std::int64_t received_timestamp_ms{0};

    bool heartbeat_present{false};
    bool heartbeat_healthy{false};
    bool position_valid{false};
    bool navigation_valid{false};
    bool attitude_valid{false};
    bool battery_valid{false};
    bool health_valid{false};
    bool mission_valid{false};

    std::optional<double> latitude_deg;
    std::optional<double> longitude_deg;
    std::optional<double> altitude_m;
    std::optional<double> ground_speed_mps;
    std::optional<double> heading_deg;
    std::optional<double> roll_rad;
    std::optional<double> pitch_rad;
    std::optional<double> yaw_rad;
    std::optional<double> battery_percent;
    std::optional<std::string> flight_mode;
    std::optional<std::string> mission_state;
};

struct MavlinkTelemetryReplay {
    std::string replay_id;
    std::vector<MavlinkTelemetrySample> samples;
};

class MavlinkTelemetryMapping final {
public:
    static NormalizedTelemetryState map(
        const MavlinkTelemetrySample& sample);

    static NormalizedTelemetryState replayLastUsable(
        const MavlinkTelemetryReplay& replay);

private:
    static TelemetryFreshness classifyFreshness(
        std::int64_t source_timestamp_ms,
        std::int64_t received_timestamp_ms);
};

} // namespace bluesky::operations
