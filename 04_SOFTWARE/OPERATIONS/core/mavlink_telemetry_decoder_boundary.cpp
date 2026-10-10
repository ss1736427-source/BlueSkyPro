#include "mavlink_telemetry_decoder_boundary.hpp"

namespace bluesky::operations {

std::optional<MavlinkTelemetrySample>
MavlinkTelemetryDecoderBoundary::decode(const DecodedMavlinkMessage& message) {
    if (message.vehicle_id.empty() ||
        message.source_id.empty() ||
        message.system_id == 0 ||
        message.component_id == 0 ||
        message.source_timestamp_ms <= 0 ||
        message.received_timestamp_ms <= 0 ||
        message.source_timestamp_ms > message.received_timestamp_ms ||
        !message.valid) {
        return std::nullopt;
    }

    if (message.kind == MavlinkMessageKind::Timesync) {
        return std::nullopt;
    }

    MavlinkTelemetrySample sample;
    sample.vehicle_id = message.vehicle_id;
    sample.source_id = message.source_id;
    sample.autopilot =
        message.dialect == MavlinkDialect::ArduPilot ? "ArduPilot" : "PX4";
    sample.mavlink_version = "2";
    sample.system_id = message.system_id;
    sample.component_id = message.component_id;
    sample.source_timestamp_ms = message.source_timestamp_ms;
    sample.received_timestamp_ms = message.received_timestamp_ms;

    switch (message.kind) {
    case MavlinkMessageKind::Heartbeat:
        sample.heartbeat_present = true;
        sample.heartbeat_healthy = message.healthy;
        sample.flight_mode = message.flight_mode;
        break;
    case MavlinkMessageKind::GlobalPositionInt:
        sample.position_valid =
            message.latitude_deg.has_value() &&
            message.longitude_deg.has_value() &&
            message.altitude_m.has_value();
        sample.latitude_deg = message.latitude_deg;
        sample.longitude_deg = message.longitude_deg;
        sample.altitude_m = message.altitude_m;
        sample.ground_speed_mps = message.ground_speed_mps;
        sample.heading_deg = message.heading_deg;
        sample.navigation_valid = sample.position_valid &&
                                  message.ground_speed_mps.has_value() &&
                                  message.heading_deg.has_value();
        break;
    case MavlinkMessageKind::Attitude:
        sample.attitude_valid =
            message.roll_rad.has_value() &&
            message.pitch_rad.has_value() &&
            message.yaw_rad.has_value();
        sample.roll_rad = message.roll_rad;
        sample.pitch_rad = message.pitch_rad;
        sample.yaw_rad = message.yaw_rad;
        break;
    case MavlinkMessageKind::SysStatus:
        sample.battery_valid = message.battery_percent.has_value();
        sample.battery_percent = message.battery_percent;
        sample.health_valid = message.healthy;
        break;
    case MavlinkMessageKind::MissionState:
        sample.mission_valid = message.mission_state.has_value();
        sample.mission_state = message.mission_state;
        break;
    case MavlinkMessageKind::Timesync:
        return std::nullopt;
    }

    return sample;
}

} // namespace bluesky::operations
