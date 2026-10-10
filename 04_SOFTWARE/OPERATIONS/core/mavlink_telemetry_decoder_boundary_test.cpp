#include "mavlink_telemetry_decoder_boundary.hpp"

#ifdef NDEBUG
#undef NDEBUG
#endif

#include <cassert>
#include <iostream>

using namespace bluesky::operations;

namespace {

DecodedMavlinkMessage base(MavlinkDialect dialect, MavlinkMessageKind kind) {
    DecodedMavlinkMessage m;
    m.dialect = dialect;
    m.kind = kind;
    m.vehicle_id = dialect == MavlinkDialect::ArduPilot ? "AP-UAV-024" : "PX-UAV-024";
    m.source_id = dialect == MavlinkDialect::ArduPilot
        ? "ArduPilot:MAVLink2:1:1"
        : "PX4:MAVLink2:1:1";
    m.system_id = 1;
    m.component_id = 1;
    m.source_timestamp_ms = 1000;
    m.received_timestamp_ms = 1000;
    m.valid = true;
    m.healthy = true;
    return m;
}

void testArduPilotHeartbeat() {
    auto m = base(MavlinkDialect::ArduPilot, MavlinkMessageKind::Heartbeat);
    m.flight_mode = "AUTO";
    const auto decoded = MavlinkTelemetryDecoderBoundary::decode(m);
    assert(decoded.has_value());
    assert(decoded->autopilot == "ArduPilot");
    assert(decoded->heartbeat_present);
    assert(decoded->system_id == 1);
    assert(decoded->component_id == 1);
}

void testPx4Position() {
    auto m = base(MavlinkDialect::PX4, MavlinkMessageKind::GlobalPositionInt);
    m.latitude_deg = 60.1699;
    m.longitude_deg = 24.9384;
    m.altitude_m = 82.0;
    m.ground_speed_mps = 18.5;
    m.heading_deg = 91.0;
    const auto decoded = MavlinkTelemetryDecoderBoundary::decode(m);
    assert(decoded.has_value());
    assert(decoded->autopilot == "PX4");
    assert(decoded->position_valid);
    assert(decoded->navigation_valid);
}

void testCommonAttitudeAndBattery() {
    auto attitude = base(MavlinkDialect::ArduPilot, MavlinkMessageKind::Attitude);
    attitude.roll_rad = 0.01;
    attitude.pitch_rad = -0.02;
    attitude.yaw_rad = 1.58;
    const auto a = MavlinkTelemetryDecoderBoundary::decode(attitude);
    assert(a.has_value() && a->attitude_valid);

    auto battery = base(MavlinkDialect::PX4, MavlinkMessageKind::SysStatus);
    battery.battery_percent = 76.0;
    const auto b = MavlinkTelemetryDecoderBoundary::decode(battery);
    assert(b.has_value() && b->battery_valid);
}

void testInvalidIdentityAndTimestampRejected() {
    auto m = base(MavlinkDialect::PX4, MavlinkMessageKind::Heartbeat);
    m.system_id = 0;
    assert(!MavlinkTelemetryDecoderBoundary::decode(m).has_value());

    m = base(MavlinkDialect::PX4, MavlinkMessageKind::Heartbeat);
    m.source_timestamp_ms = 2000;
    m.received_timestamp_ms = 1000;
    assert(!MavlinkTelemetryDecoderBoundary::decode(m).has_value());

    m = base(MavlinkDialect::PX4, MavlinkMessageKind::Heartbeat);
    m.valid = false;
    assert(!MavlinkTelemetryDecoderBoundary::decode(m).has_value());
}

void testMissionFieldRemainsTypedBoundaryOnly() {
    auto m = base(MavlinkDialect::ArduPilot, MavlinkMessageKind::MissionState);
    m.mission_state = "ACTIVE";
    const auto decoded = MavlinkTelemetryDecoderBoundary::decode(m);
    assert(decoded.has_value());
    assert(decoded->mission_valid);
    assert(decoded->mission_state == "ACTIVE");
}

} // namespace

int main() {
    testArduPilotHeartbeat();
    testPx4Position();
    testCommonAttitudeAndBattery();
    testInvalidIdentityAndTimestampRejected();
    testMissionFieldRemainsTypedBoundaryOnly();
    std::cout << "mavlink_telemetry_decoder_boundary_test: PASS\n";
    return 0;
}
