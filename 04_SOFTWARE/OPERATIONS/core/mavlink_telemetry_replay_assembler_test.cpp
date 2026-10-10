#include "mavlink_telemetry_replay_assembler.hpp"
#include "mavlink_telemetry_mapping.hpp"

#ifdef NDEBUG
#undef NDEBUG
#endif

#include <cassert>
#include <iostream>
#include <vector>

using namespace bluesky::operations;

namespace {

DecodedMavlinkMessage base(MavlinkDialect dialect, MavlinkMessageKind kind, std::int64_t t) {
    DecodedMavlinkMessage m;
    m.dialect = dialect;
    m.kind = kind;
    m.vehicle_id = dialect == MavlinkDialect::ArduPilot ? "AP-UAV-025" : "PX-UAV-025";
    m.source_id = dialect == MavlinkDialect::ArduPilot
        ? "ArduPilot:MAVLink2:1:1"
        : "PX4:MAVLink2:1:1";
    m.system_id = 1;
    m.component_id = 1;
    m.source_timestamp_ms = t;
    m.received_timestamp_ms = t;
    m.valid = true;
    m.healthy = true;
    return m;
}

void testArduPilotReplayBuildsUsableSnapshot() {
    std::vector<DecodedMavlinkMessage> messages;

    auto hb = base(MavlinkDialect::ArduPilot, MavlinkMessageKind::Heartbeat, 1000);
    hb.flight_mode = "AUTO";
    messages.push_back(hb);

    auto pos = base(MavlinkDialect::ArduPilot, MavlinkMessageKind::GlobalPositionInt, 1100);
    pos.latitude_deg = 60.1;
    pos.longitude_deg = 24.9;
    pos.altitude_m = 82.0;
    pos.ground_speed_mps = 18.0;
    pos.heading_deg = 91.0;
    messages.push_back(pos);

    auto att = base(MavlinkDialect::ArduPilot, MavlinkMessageKind::Attitude, 1200);
    att.roll_rad = 0.01;
    att.pitch_rad = -0.02;
    att.yaw_rad = 1.58;
    messages.push_back(att);

    auto bat = base(MavlinkDialect::ArduPilot, MavlinkMessageKind::SysStatus, 1300);
    bat.battery_percent = 76.0;
    messages.push_back(bat);

    const auto replay = MavlinkTelemetryReplayAssembler::assemble("AP-REPLAY-025", messages);
    assert(replay.samples.size() == 1);
    const auto state = MavlinkTelemetryMapping::replayLastUsable(replay);
    assert(normalized_state_usable(state));
    assert(state.vehicle_id == "AP-UAV-025");
    assert(state.source_timestamp_ms == 1300);
}

void testPx4ReplayRejectsIncompleteSnapshot() {
    std::vector<DecodedMavlinkMessage> messages;

    auto hb = base(MavlinkDialect::PX4, MavlinkMessageKind::Heartbeat, 1000);
    hb.flight_mode = "AUTO";
    messages.push_back(hb);

    auto pos = base(MavlinkDialect::PX4, MavlinkMessageKind::GlobalPositionInt, 1100);
    pos.latitude_deg = 60.1;
    pos.longitude_deg = 24.9;
    pos.altitude_m = 82.0;
    messages.push_back(pos);

    const auto replay = MavlinkTelemetryReplayAssembler::assemble("PX-REPLAY-025", messages);
    assert(replay.samples.size() == 1);
    const auto state = MavlinkTelemetryMapping::replayLastUsable(replay);
    assert(!normalized_state_usable(state));
}

void testLatestUnhealthyHeartbeatOverridesEarlierHealthyHeartbeat() {
    std::vector<DecodedMavlinkMessage> messages;

    auto healthy = base(MavlinkDialect::ArduPilot, MavlinkMessageKind::Heartbeat, 1000);
    healthy.flight_mode = "AUTO";
    messages.push_back(healthy);

    auto pos = base(MavlinkDialect::ArduPilot, MavlinkMessageKind::GlobalPositionInt, 1100);
    pos.latitude_deg = 60.1;
    pos.longitude_deg = 24.9;
    pos.altitude_m = 82.0;
    pos.ground_speed_mps = 18.0;
    pos.heading_deg = 91.0;
    messages.push_back(pos);

    auto att = base(MavlinkDialect::ArduPilot, MavlinkMessageKind::Attitude, 1200);
    att.roll_rad = 0.01;
    att.pitch_rad = -0.02;
    att.yaw_rad = 1.58;
    messages.push_back(att);

    auto unhealthy = base(MavlinkDialect::ArduPilot, MavlinkMessageKind::Heartbeat, 1300);
    unhealthy.healthy = false;
    unhealthy.flight_mode = "AUTO";
    messages.push_back(unhealthy);

    const auto replay = MavlinkTelemetryReplayAssembler::assemble("AP-HEALTH-DEGRADE", messages);
    assert(replay.samples.size() == 1);
    const auto state = MavlinkTelemetryMapping::replayLastUsable(replay);
    assert(!state.c2_state_valid);
    assert(!state.vehicle_state_valid);
    assert(!normalized_state_usable(state));
}

void testInvalidMessageIsExcluded() {
    std::vector<DecodedMavlinkMessage> messages;
    auto invalid = base(MavlinkDialect::PX4, MavlinkMessageKind::Heartbeat, 1000);
    invalid.valid = false;
    messages.push_back(invalid);

    const auto replay = MavlinkTelemetryReplayAssembler::assemble("INVALID-025", messages);
    assert(replay.samples.empty());
}

} // namespace

int main() {
    testArduPilotReplayBuildsUsableSnapshot();
    testPx4ReplayRejectsIncompleteSnapshot();
    testLatestUnhealthyHeartbeatOverridesEarlierHealthyHeartbeat();
    testInvalidMessageIsExcluded();
    std::cout << "mavlink_telemetry_replay_assembler_test: PASS\n";
    return 0;
}
