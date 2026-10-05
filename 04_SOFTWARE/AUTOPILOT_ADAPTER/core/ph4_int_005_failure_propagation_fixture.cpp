#include "autopilot_adapter_baseline.hpp"
#include "c2_interface_baseline.hpp"

#include <cassert>
#include <iostream>
#include <string>

int main() {
    using namespace bluesky::autopilot;
    using namespace bluesky::c2;

    AutopilotAdapterBaseline adapter(Identity{"FIXTURE-UAV-001", "FIXTURE", "FIXTURE-AUTOPILOT", "1.0", "FIXTURE", "1.0"});
    C2InterfaceBaseline c2(C2Identity{"FIXTURE-UAV-001", "FIXTURE-TRANSPORT", "FIXTURE-PROTOCOL", "1.0"});

    auto disconnected = adapter.startMission();
    assert(!disconnected.acknowledged);
    assert(disconnected.executionState == ExecutionState::Rejected);
    assert(disconnected.error == ErrorCode::NotConnected);
    assert(disconnected.reason == "AUTOPILOT_NOT_CONNECTED");

    assert(adapter.connect());
    const std::string approved = "MISSION-FIXTURE-005";
    assert(adapter.write(approved).acknowledged);

    const auto actual = adapter.readBack();
    assert(actual.has_value());
    const auto match = adapter.compare(approved, *actual);
    assert(match.equal);

    const std::string altered = approved + ":ALTERED";
    const auto mismatch = adapter.compare(approved, altered);
    assert(!mismatch.equal);
    assert(mismatch.mismatches.size() == 1);
    assert(mismatch.mismatches.front() == "MISSION_CONTENT_MISMATCH");

    const auto invalid = adapter.write("");
    assert(!invalid.acknowledged);
    assert(invalid.executionState == ExecutionState::Rejected);
    assert(invalid.error == ErrorCode::InvalidState);
    assert(invalid.reason == "CONFIGURATION_REQUIRED");

    auto c2Disconnected = c2.send("MISSION", approved);
    assert(!c2Disconnected.accepted);
    assert(c2Disconnected.error == C2ErrorCode::NotConnected);
    assert(c2Disconnected.reason == "C2_LINK_NOT_CONNECTED");

    assert(c2.connect());
    auto c2Auth = c2.authenticate("");
    assert(!c2Auth.accepted);
    assert(c2Auth.error == C2ErrorCode::AuthenticationFailed);
    assert(c2Auth.reason == "CREDENTIAL_REFERENCE_REQUIRED");

    auto c2Heartbeat = c2.heartbeat(5000);
    assert(c2Heartbeat.accepted);
    assert(c2Heartbeat.reason == "HEARTBEAT_OK");

    std::cout << "PH4_INT_005_FAILURE_PROPAGATION_FIXTURE: PASS\n";
    std::cout << "command_rejection_propagation=PASS\n";
    std::cout << "readback_mismatch_blocking=PASS\n";
    std::cout << "invalid_write_reason_preserved=PASS\n";
    std::cout << "c2_error_propagation=PASS\n";
    std::cout << "verification_status=FIXTURE_ONLY\n";
}
