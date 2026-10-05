#include "autopilot_adapter_baseline.hpp"

#include <algorithm>
#include <iostream>
#include <string>

namespace {

bool hasCapability(const bluesky::autopilot::Capabilities& capabilities, const std::string& value) {
    return std::find(capabilities.missionFeatures.begin(), capabilities.missionFeatures.end(), value) != capabilities.missionFeatures.end();
}

} // namespace

int main() {
    using namespace bluesky::autopilot;

    Identity identity;
    identity.vehicleId = "UAV-LOG-024";
    identity.vehicleType = "SIL-UAV";
    identity.autopilot = "SIL-FCS";
    identity.firmwareVersion = "FIXTURE-1";
    identity.protocol = "MAVLINK2";
    identity.protocolVersion = "BASELINE-1";

    AutopilotAdapterBaseline adapter(identity);
    const auto capabilities = adapter.getCapabilities();

    const bool capability = hasCapability(capabilities, "flight_log_acquisition");
    const auto disconnected = adapter.acquireFlightLog("LOG-024-A");
    const bool disconnectedRejected = !disconnected.accepted && disconnected.error == ErrorCode::NotConnected;

    adapter.connect();
    const auto acquired = adapter.acquireFlightLog("LOG-024-A");
    const bool success = acquired.accepted && acquired.executionState == ExecutionState::Completed
        && acquired.error == ErrorCode::None
        && acquired.logId == "LOG-024-A"
        && acquired.vehicleId == "UAV-LOG-024"
        && acquired.sourceReference == "SIL_LOG_SOURCE:LOG-024-A";

    const auto invalid = adapter.acquireFlightLog("");
    const bool invalidRejected = !invalid.accepted && invalid.error == ErrorCode::InvalidState;

    adapter.disconnect();
    const auto afterDisconnect = adapter.acquireFlightLog("LOG-024-B");
    const bool postDisconnectRejected = !afterDisconnect.accepted && afterDisconnect.error == ErrorCode::NotConnected;

    std::cout
        << "PH4_INT_024|FLIGHT-RECORD-PH4-INT-024|UAV-LOG-024|MAVLINK2|BASELINE-1"
        << "|CAPABILITY=" << (capability ? "PASS" : "FAIL")
        << "|DISCONNECTED_REJECTED=" << (disconnectedRejected ? "PASS" : "FAIL")
        << "|ACQUIRED=" << (success ? "PASS" : "FAIL")
        << "|INVALID_LOG_ID=" << (invalidRejected ? "PASS" : "FAIL")
        << "|POST_DISCONNECT_REJECTED=" << (postDisconnectRejected ? "PASS" : "FAIL")
        << "|PROVENANCE=" << ((acquired.sourceReference == "SIL_LOG_SOURCE:LOG-024-A") ? "PASS" : "FAIL")
        << "|STATE_BOUNDARY=" << ((acquired.executionState == ExecutionState::Completed && afterDisconnect.executionState == ExecutionState::Rejected) ? "PASS" : "FAIL")
        << std::endl;

    return capability && disconnectedRejected && success && invalidRejected && postDisconnectRejected ? 0 : 1;
}
