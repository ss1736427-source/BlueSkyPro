#include "autopilot_adapter_baseline.hpp"
#include <algorithm>
#include <iostream>
#include <string>

int main() {
    using namespace bluesky::autopilot;
    Identity identity;
    identity.vehicleId = "UAV-CLOSE-025";
    identity.vehicleType = "SIL-UAV";
    identity.autopilot = "SIL-FCS";
    identity.firmwareVersion = "FIXTURE-1";
    identity.protocol = "MAVLINK2";
    identity.protocolVersion = "BASELINE-1";
    AutopilotAdapterBaseline adapter(identity);
    const auto capabilities = adapter.getCapabilities();
    const bool capability = std::find(capabilities.missionFeatures.begin(), capabilities.missionFeatures.end(), "clean_close") != capabilities.missionFeatures.end();
    adapter.connect();
    const auto firstClose = adapter.close();
    const bool closed = firstClose.acknowledged && firstClose.executionState == ExecutionState::Completed && firstClose.error == ErrorCode::None && firstClose.reason == "CLOSED_AND_ARCHIVE_READY";
    const bool disconnected = adapter.getConnectionState() == ConnectionState::Disconnected && !adapter.getLinkMetrics().healthy;
    const auto repeatClose = adapter.close();
    const bool idempotent = repeatClose.acknowledged && repeatClose.executionState == ExecutionState::Completed && repeatClose.error == ErrorCode::None && repeatClose.reason == "ALREADY_CLOSED";
    const auto commandAfterClose = adapter.hold();
    const bool executionBlocked = !commandAfterClose.acknowledged && commandAfterClose.executionState == ExecutionState::Rejected && commandAfterClose.error == ErrorCode::NotConnected;
    adapter.connect();
    const auto reopened = adapter.hold();
    const bool reconnectRestoresOperationalBoundary = reopened.acknowledged && reopened.executionState == ExecutionState::Acknowledged;
    const auto finalClose = adapter.close();
    const bool finalClosed = finalClose.acknowledged && finalClose.executionState == ExecutionState::Completed;
    std::cout << "PH4_INT_025|FLIGHT-RECORD-PH4-INT-025|UAV-CLOSE-025|MAVLINK2|BASELINE-1"
              << "|CAPABILITY=" << (capability ? "PASS" : "FAIL")
              << "|CLOSED=" << (closed ? "PASS" : "FAIL")
              << "|DISCONNECTED=" << (disconnected ? "PASS" : "FAIL")
              << "|IDEMPOTENT=" << (idempotent ? "PASS" : "FAIL")
              << "|EXECUTION_BLOCKED=" << (executionBlocked ? "PASS" : "FAIL")
              << "|RECONNECT_BOUNDARY=" << (reconnectRestoresOperationalBoundary ? "PASS" : "FAIL")
              << "|FINAL_CLOSED=" << (finalClosed ? "PASS" : "FAIL")
              << "|STATE_BOUNDARY=" << ((finalClose.executionState == ExecutionState::Completed && adapter.getConnectionState() == ConnectionState::Disconnected) ? "PASS" : "FAIL")
              << std::endl;
    return capability && closed && disconnected && idempotent && executionBlocked && reconnectRestoresOperationalBoundary && finalClosed ? 0 : 1;
}
