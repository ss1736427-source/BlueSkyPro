#include "vehicle_state_mapping.hpp"
#include "../../AUTOPILOT_ADAPTER/core/autopilot_adapter_baseline.hpp"

#ifdef NDEBUG
#undef NDEBUG
#endif

#include <cassert>
#include <iostream>

using namespace bluesky::operations;

namespace {

bluesky::autopilot::AutopilotAdapterBaseline makeAdapter() {
    bluesky::autopilot::Identity identity;
    identity.vehicleId = "UAV-022";
    identity.vehicleType = "TEST";
    identity.autopilot = "TEST-FCS";
    identity.firmwareVersion = "FW-1";
    identity.protocol = "MAVLink2";
    identity.protocolVersion = "2";
    return bluesky::autopilot::AutopilotAdapterBaseline(identity);
}

void testValidFrame() {
    VehicleStateTelemetryFrame frame;
    frame.vehicle_id = "UAV-022";
    frame.source_id = "PX4:MAVLink2";
    frame.adapter_version = "2";
    frame.source_timestamp_ms = 1000;
    frame.received_timestamp_ms = 1000;
    frame.position_valid = true;
    frame.navigation_valid = true;
    frame.vehicle_state_valid = true;
    frame.energy_state_valid = true;
    frame.c2_state_valid = true;

    const auto state = VehicleStateMapping::fromFrame(frame);
    assert(state.freshness == TelemetryFreshness::Current);
    assert(normalized_state_usable(state));
    assert(state.vehicle_id == "UAV-022");
}

void testStaleOrDelayedFrameIsNotUsable() {
    VehicleStateTelemetryFrame frame;
    frame.vehicle_id = "UAV-022";
    frame.source_id = "PX4:MAVLink2";
    frame.adapter_version = "2";
    frame.source_timestamp_ms = 900;
    frame.received_timestamp_ms = 1000;
    frame.position_valid = true;
    frame.navigation_valid = true;
    frame.vehicle_state_valid = true;

    const auto state = VehicleStateMapping::fromFrame(frame);
    assert(state.freshness == TelemetryFreshness::Delayed);
    assert(!normalized_state_usable(state));
}

void testMissingTimestampsForceMissing() {
    VehicleStateTelemetryFrame frame;
    frame.vehicle_id = "UAV-022";
    frame.source_id = "PX4:MAVLink2";
    frame.position_valid = true;
    frame.navigation_valid = true;
    frame.vehicle_state_valid = true;

    const auto state = VehicleStateMapping::fromFrame(frame);
    assert(state.freshness == TelemetryFreshness::Missing);
    assert(!normalized_state_usable(state));
}

void testAdapterBoundaryUsesNormalizedContract() {
    auto adapter = makeAdapter();
    assert(adapter.connect());

    const auto state = VehicleStateMapping::fromAdapter(adapter, 1000, 1000);
    assert(state.vehicle_id == "UAV-022");
    assert(state.provenance.source_id == "TEST-FCS:MAVLink2");
    assert(state.c2_state_valid);
    assert(!normalized_state_usable(state));
}

} // namespace

int main() {
    testValidFrame();
    testStaleOrDelayedFrameIsNotUsable();
    testMissingTimestampsForceMissing();
    testAdapterBoundaryUsesNormalizedContract();
    std::cout << "vehicle_state_mapping_test: PASS\n";
    return 0;
}
