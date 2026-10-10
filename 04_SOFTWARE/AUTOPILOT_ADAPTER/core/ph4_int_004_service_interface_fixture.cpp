#include "autopilot_adapter_baseline.hpp"
#include "c2_interface_baseline.hpp"

#include <cassert>
#include <iostream>
#include <string>

int main() {
    using bluesky::autopilot::AutopilotAdapterBaseline;
    using bluesky::autopilot::ConnectionState;
    using bluesky::autopilot::ErrorCode;
    using bluesky::autopilot::ExecutionState;
    using bluesky::autopilot::Identity;
    using bluesky::c2::C2Identity;
    using bluesky::c2::C2InterfaceBaseline;

    AutopilotAdapterBaseline adapter(Identity{
        "FIXTURE-UAV-001", "FIXTURE", "FIXTURE-AUTOPILOT",
        "1.0", "FIXTURE", "1.0"});
    C2InterfaceBaseline c2(C2Identity{
        "FIXTURE-UAV-001", "FIXTURE-TRANSPORT", "FIXTURE-PROTOCOL", "1.0"});

    assert(adapter.connect());
    assert(adapter.getConnectionState() == ConnectionState::Connected);

    auto c2Connect = c2.connect();
    assert(c2Connect);
    auto auth = c2.authenticate("FIXTURE-CREDENTIAL");
    assert(auth.accepted);
    auto heartbeat = c2.heartbeat(1000);
    assert(heartbeat.accepted);

    const std::string mission = "MISSION-FIXTURE-001";
    const auto write = adapter.write(mission);
    assert(write.acknowledged);
    assert(write.executionState == ExecutionState::Acknowledged);
    assert(write.error == ErrorCode::None);

    const auto readBack = adapter.readBack();
    assert(readBack.has_value());
    assert(*readBack == mission);

    const auto comparison = adapter.compare(mission, *readBack);
    assert(comparison.equal);
    assert(comparison.mismatches.empty());

    const auto dispatch = c2.send("MISSION", mission);
    assert(dispatch.accepted);
    const auto frame = c2.receive();
    assert(frame.has_value());
    assert(frame->channel == "MISSION");
    assert(frame->payload == mission);

    const auto adapterCommand = adapter.startMission();
    assert(adapterCommand.acknowledged);
    assert(adapterCommand.executionState == ExecutionState::Acknowledged);
    assert(adapterCommand.sourceTimestampMs > 0);
    assert(adapterCommand.adapterTimestampMs >= adapterCommand.sourceTimestampMs);

    std::cout << "PH4_INT_004_SERVICE_INTERFACE_FIXTURE: PASS\n";
    std::cout << "adapter_connection=PASS\n";
    std::cout << "c2_auth_heartbeat=PASS\n";
    std::cout << "mission_write_readback_compare=PASS\n";
    std::cout << "c2_dispatch_receive=PASS\n";
    std::cout << "command_acknowledgement=PASS\n";
    std::cout << "verification_status=FIXTURE_ONLY\n";
    return 0;
}
