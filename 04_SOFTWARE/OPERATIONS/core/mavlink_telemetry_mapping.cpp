#include "mavlink_telemetry_mapping.hpp"

namespace bluesky::operations {

TelemetryFreshness MavlinkTelemetryMapping::classifyFreshness(
    std::int64_t source_timestamp_ms,
    std::int64_t received_timestamp_ms) {
    if (source_timestamp_ms <= 0 || received_timestamp_ms <= 0) {
        return TelemetryFreshness::Missing;
    }
    if (source_timestamp_ms > received_timestamp_ms) {
        return TelemetryFreshness::Delayed;
    }
    if (source_timestamp_ms == received_timestamp_ms) {
        return TelemetryFreshness::Current;
    }
    return TelemetryFreshness::Delayed;
}

NormalizedTelemetryState MavlinkTelemetryMapping::map(
    const MavlinkTelemetrySample& sample) {
    NormalizedTelemetryState state;
    state.vehicle_id = sample.vehicle_id;
    state.snapshot_version = "mavlink-telemetry-1";
    state.provenance.source_id = sample.source_id;
    state.provenance.source_type = "mavlink";
    state.provenance.adapter_version = sample.mavlink_version;
    state.source_timestamp_ms = sample.source_timestamp_ms;
    state.received_timestamp_ms = sample.received_timestamp_ms;
    state.freshness = classifyFreshness(
        sample.source_timestamp_ms,
        sample.received_timestamp_ms);

    state.position_valid = sample.position_valid &&
        sample.latitude_deg.has_value() &&
        sample.longitude_deg.has_value() &&
        sample.altitude_m.has_value();

    state.navigation_valid = sample.navigation_valid &&
        sample.position_valid &&
        sample.ground_speed_mps.has_value() &&
        sample.heading_deg.has_value();

    state.vehicle_state_valid = sample.heartbeat_present &&
        sample.heartbeat_healthy &&
        sample.attitude_valid &&
        sample.flight_mode.has_value();

    state.energy_state_valid = sample.battery_valid &&
        sample.battery_percent.has_value();

    state.c2_state_valid = sample.heartbeat_present &&
        sample.heartbeat_healthy;

    return state;
}

NormalizedTelemetryState MavlinkTelemetryMapping::replayLastUsable(
    const MavlinkTelemetryReplay& replay) {
    NormalizedTelemetryState last;
    for (const auto& sample : replay.samples) {
        const auto mapped = map(sample);
        if (normalized_state_usable(mapped)) {
            last = mapped;
        }
    }
    return last;
}

} // namespace bluesky::operations
