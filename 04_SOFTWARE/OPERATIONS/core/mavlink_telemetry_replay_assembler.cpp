#include "mavlink_telemetry_replay_assembler.hpp"

#include <algorithm>

namespace bluesky::operations {

MavlinkTelemetryReplay MavlinkTelemetryReplayAssembler::assemble(
    const std::string& replay_id,
    const std::vector<DecodedMavlinkMessage>& messages) {
    MavlinkTelemetryReplay replay;
    replay.replay_id = replay_id;

    MavlinkTelemetrySample current;
    bool has_current = false;

    for (const auto& message : messages) {
        const auto decoded = MavlinkTelemetryDecoderBoundary::decode(message);
        if (!decoded.has_value()) {
            continue;
        }

        auto& sample = *decoded;
        if (!has_current ||
            sample.vehicle_id != current.vehicle_id ||
            sample.source_id != current.source_id ||
            sample.system_id != current.system_id ||
            sample.component_id != current.component_id) {
            if (has_current) {
                replay.samples.push_back(current);
            }
            current = sample;
            has_current = true;
            continue;
        }

        current.source_timestamp_ms =
            std::max(current.source_timestamp_ms, sample.source_timestamp_ms);
        current.received_timestamp_ms =
            std::max(current.received_timestamp_ms, sample.received_timestamp_ms);

        // Health is state, not an accumulating capability: the latest heartbeat
        // for this identity must be able to report degradation.
        if (sample.heartbeat_present) {
            current.heartbeat_present = true;
            current.heartbeat_healthy = sample.heartbeat_healthy;
            if (sample.flight_mode) current.flight_mode = sample.flight_mode;
        }
        current.position_valid |= sample.position_valid;
        current.navigation_valid |= sample.navigation_valid;
        current.attitude_valid |= sample.attitude_valid;
        current.battery_valid |= sample.battery_valid;
        current.health_valid |= sample.health_valid;
        current.mission_valid |= sample.mission_valid;

        if (sample.latitude_deg) current.latitude_deg = sample.latitude_deg;
        if (sample.longitude_deg) current.longitude_deg = sample.longitude_deg;
        if (sample.altitude_m) current.altitude_m = sample.altitude_m;
        if (sample.ground_speed_mps) current.ground_speed_mps = sample.ground_speed_mps;
        if (sample.heading_deg) current.heading_deg = sample.heading_deg;
        if (sample.roll_rad) current.roll_rad = sample.roll_rad;
        if (sample.pitch_rad) current.pitch_rad = sample.pitch_rad;
        if (sample.yaw_rad) current.yaw_rad = sample.yaw_rad;
        if (sample.battery_percent) current.battery_percent = sample.battery_percent;
        if (sample.mission_state) current.mission_state = sample.mission_state;
    }

    if (has_current) {
        replay.samples.push_back(current);
    }
    return replay;
}

} // namespace bluesky::operations
