#include "ardupilot_mavlink2_reference_adapter.hpp"

#include <iostream>

int main() {
    using namespace bluesky::autopilot;

    Identity identity;
    identity.vehicleId = "UAV-AP-001";
    identity.vehicleType = "REFERENCE-UAV";
    identity.firmwareVersion = "REFERENCE-FIXTURE-1";
    identity.protocolVersion = "MAVLINK2";

    ArduPilotMavlink2ReferenceAdapter adapter(identity);
    const auto reported = adapter.getVehicleIdentity();
    const auto capabilities = adapter.getCapabilities();

    const bool identityOk = reported.vehicleId == "UAV-AP-001"
        && reported.autopilot == "ArduPilot"
        && reported.protocol == "MAVLINK2";
    const bool capabilityOk = !capabilities.commands.empty()
        && !capabilities.missionFeatures.empty()
        && !capabilities.equipmentFeatures.empty();
    const bool universalBoundaryOk = adapter.getAutopilotIdentity() == "ArduPilot"
        && adapter.getProtocolVersion() == "MAVLINK2";

    std::cout << "PH5_AP_001|UAV-AP-001|ArduPilot|MAVLINK2"
              << "|IDENTITY=" << (identityOk ? "PASS" : "FAIL")
              << "|CAPABILITIES=" << (capabilityOk ? "PASS" : "FAIL")
              << "|UNIVERSAL_BOUNDARY=" << (universalBoundaryOk ? "PASS" : "FAIL")
              << "|TRANSPORT=SIL_BOUNDARY_ONLY"
              << std::endl;

    return identityOk && capabilityOk && universalBoundaryOk ? 0 : 1;
}
