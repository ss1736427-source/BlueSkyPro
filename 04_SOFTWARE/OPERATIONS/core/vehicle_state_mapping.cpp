#include "vehicle_state_mapping.hpp"

namespace bluesky::operations {

TelemetryFreshness VehicleStateMapping::classifyFreshness(
    std::int64_t source_timestamp_ms,
    std::int64_t received_timestamp_ms) {
    if (source_timestamp_ms <= 0 || received_timestamp_ms <= 0) {
        return TelemetryFreshness::Missing;
    }
    if (source_timestamp_ms > received_timestamp_ms) {
        return TelemetryFreshness::Delayed;
    }
    const auto age_ms = received_timestamp_ms - source_timestamp_ms;
    if (age_ms == 0) {
        return TelemetryFreshness::Current;
    }
    return TelemetryFreshness::Delayed;
}

NormalizedTelemetryState VehicleStateMapping::fromFrame(
    const VehicleStateTelemetryFrame& frame) {
    NormalizedTelemetryState state;
    state.vehicle_id = frame.vehicle_id;
    state.snapshot_version = frame.snapshot_version;
    state.provenance.source_id = frame.source_id;
    state.provenance.source_type = frame.source_type;
    state.provenance.adapter_version = frame.adapter_version;
    state.freshness = classifyFreshness(
        frame.source_timestamp_ms, frame.received_timestamp_ms);
    state.source_timestamp_ms = frame.source_timestamp_ms;
    state.received_timestamp_ms = frame.received_timestamp_ms;
    state.position_valid = frame.position_valid;
    state.navigation_valid = frame.navigation_valid;
    state.vehicle_state_valid = frame.vehicle_state_valid;
    state.energy_state_valid = frame.energy_state_valid;
    state.c2_state_valid = frame.c2_state_valid;
    return state;
}

NormalizedTelemetryState VehicleStateMapping::fromAdapter(
    const bluesky::autopilot::UniversalAutopilotAdapter& adapter,
    std::int64_t source_timestamp_ms,
    std::int64_t received_timestamp_ms) {
    VehicleStateTelemetryFrame frame;
    const auto identity = adapter.getVehicleIdentity();
    frame.vehicle_id = identity.vehicleId;
    frame.source_id = identity.autopilot + ":" + identity.protocol;
    frame.source_type = "autopilot";
    frame.adapter_version = identity.protocolVersion;
    frame.source_timestamp_ms = source_timestamp_ms;
    frame.received_timestamp_ms = received_timestamp_ms;
    frame.position_valid =
        adapter.getNavigationState().value != "UNKNOWN";
    frame.navigation_valid =
        adapter.getNavigationState().value != "UNKNOWN";
    frame.vehicle_state_valid =
        adapter.getFlightMode().value != "UNKNOWN" &&
        adapter.getHealthState().value != "UNKNOWN";
    frame.energy_state_valid =
        adapter.getEnergyState().value != "UNKNOWN";
    frame.c2_state_valid =
        adapter.getConnectionState() == bluesky::autopilot::ConnectionState::Connected ||
        adapter.getConnectionState() == bluesky::autopilot::ConnectionState::Degraded;
    return fromFrame(frame);
}

} // namespace bluesky::operations
