#include "mavlink_telemetry_mapping.hpp"

#ifdef NDEBUG
#undef NDEBUG
#endif

#include <cassert>
#include <iostream>

using namespace bluesky::operations;

namespace {

MavlinkTelemetrySample makeValidSample(std::int64_t timestamp) {
    MavlinkTelemetrySample sample;
    sample.vehicle_id = "UAV-023";
    sample.source_id = "PX4:MAVLink2:1:1";
    sample.autopilot = "PX4";
    sample.system_id = 1;
    sample.component_id = 1;
    sample.source_timestamp_ms = timestamp;
    sample.received_timestamp_ms = timestamp;
    sample.heartbeat_present = true;
    sample.heartbeat_healthy = true;
    sample.position_valid = true;
    sample.navigation_valid = true;
    sample.attitude_valid = true;
    sample.battery_valid = true;
    sample.health_valid = true;
    sample.mission_valid = true;
    sample.latitude_deg = 60.1699;
    sample.longitude_deg = 24.9384;
    sample.altitude_m = 82.0;
    sample.ground_speed_mps = 18.5;
    sample.heading_deg = 91.0;
    sample.roll_rad = 0.01;
    sample.pitch_rad = -0.02;
    sample.yaw_rad = 1.58;
    sample.battery_percent = 76.0;
    sample.flight_mode = "AUTO";
    sample.mission_state = "ACTIVE";
    return sample;
}

void testTypedSampleMapsToCanonicalState() {
    const auto state = MavlinkTelemetryMapping::map(makeValidSample(2000));
    assert(state.freshness == TelemetryFreshness::Current);
    assert(state.vehicle_id == "UAV-023");
    assert(state.provenance.source_id == "PX4:MAVLink2:1:1");
    assert(state.position_valid);
    assert(state.navigation_valid);
    assert(state.vehicle_state_valid);
    assert(state.energy_state_valid);
    assert(state.c2_state_valid);
    assert(normalized_state_usable(state));
}

void testHeartbeatLossBlocksUsability() {
    auto sample = makeValidSample(3000);
    sample.heartbeat_healthy = false;
    const auto state = MavlinkTelemetryMapping::map(sample);
    assert(!state.c2_state_valid);
    assert(!state.vehicle_state_valid);
    assert(!normalized_state_usable(state));
}

void testIncompletePositionIsRejected() {
    auto sample = makeValidSample(4000);
    sample.longitude_deg.reset();
    const auto state = MavlinkTelemetryMapping::map(sample);
    assert(!state.position_valid);
    assert(!normalized_state_usable(state));
}

void testReplaySelectsLastUsableTypedFrame() {
    MavlinkTelemetryReplay replay;
    replay.replay_id = "PX4-SIL-REPLAY-023";
    replay.samples.push_back(makeValidSample(1000));

    auto delayed = makeValidSample(2000);
    delayed.received_timestamp_ms = 2100;
    replay.samples.push_back(delayed);

    auto recovered = makeValidSample(3000);
    replay.samples.push_back(recovered);

    const auto state = MavlinkTelemetryMapping::replayLastUsable(replay);
    assert(state.freshness == TelemetryFreshness::Current);
    assert(state.source_timestamp_ms == 3000);
}

void testMissingTimestampIsNeverUsable() {
    auto sample = makeValidSample(0);
    sample.received_timestamp_ms = 0;
    const auto state = MavlinkTelemetryMapping::map(sample);
    assert(state.freshness == TelemetryFreshness::Missing);
    assert(!normalized_state_usable(state));
}

} // namespace

int main() {
    testTypedSampleMapsToCanonicalState();
    testHeartbeatLossBlocksUsability();
    testIncompletePositionIsRejected();
    testReplaySelectsLastUsableTypedFrame();
    testMissingTimestampIsNeverUsable();
    std::cout << "mavlink_telemetry_mapping_test: PASS\n";
    return 0;
}
