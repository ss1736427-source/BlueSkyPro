#pragma once

#include "mavlink_telemetry_mapping.hpp"

#include <cstdint>
#include <optional>
#include <string>

namespace bluesky::operations {

enum class MavlinkDialect {
    ArduPilot,
    PX4
};

enum class MavlinkMessageKind {
    Heartbeat,
    GlobalPositionInt,
    Attitude,
    SysStatus,
    MissionState,
    Timesync
};

struct DecodedMavlinkMessage {
    MavlinkDialect dialect{MavlinkDialect::PX4};
    MavlinkMessageKind kind{MavlinkMessageKind::Heartbeat};
    std::string vehicle_id;
    std::string source_id;
    std::uint8_t system_id{0};
    std::uint8_t component_id{0};
    std::int64_t source_timestamp_ms{0};
    std::int64_t received_timestamp_ms{0};

    bool healthy{false};
    bool valid{false};
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

class MavlinkTelemetryDecoderBoundary final {
public:
    static std::optional<MavlinkTelemetrySample> decode(
        const DecodedMavlinkMessage& message);
};

} // namespace bluesky::operations
