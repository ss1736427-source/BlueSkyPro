#include "autopilot_adapter_baseline.hpp"
#include "../../PLANNING/model/mission_model.hpp"
#include "../../PLANNING/model/vehicle_equipment_capability.hpp"
#include <iostream>
#include <string>

int main() {
    using namespace bluesky::autopilot;
    using namespace bluesky::planning;

    Identity identity;
    identity.vehicleId = "UAV-E2E-027";
    identity.vehicleType = "SIL-UAV";
    identity.autopilot = "SIL-FCS";
    identity.firmwareVersion = "FIXTURE-1";
    identity.protocol = "MAVLINK2";
    identity.protocolVersion = "BASELINE-1";

    AutopilotAdapterBaseline adapter(identity);

    Mission mission;
    mission.identity.mission_id = "MISSION-027";
    mission.identity.mission_version = "1";
    mission.identity.mission_type = "E2E_ADAPTER_GATE";
    mission.identity.operational_status = "RELEASED";
    mission.vehicle_assignments.push_back({"UAV-E2E-027", "CFG-027"});

    VehicleEquipmentCapabilityProfile profile;
    profile.identity.vehicle_id = "UAV-E2E-027";
    profile.identity.autopilot_type = "SIL-FCS";
    profile.identity.autopilot_version = "FIXTURE-1";
    profile.configuration.configuration_version = "CFG-027";
    profile.configuration.valid_for_mission = true;

    const auto capabilities = adapter.getCapabilities();
    const bool capabilityGate = !capabilities.commands.empty() && !capabilities.missionFeatures.empty();
    const bool connected = adapter.connect() && adapter.getConnectionState() == ConnectionState::Connected;
    const auto reportedIdentity = adapter.getVehicleIdentity();
    const bool identified = reportedIdentity.vehicleId == "UAV-E2E-027" && reportedIdentity.protocol == "MAVLINK2";

    const auto compiled = adapter.compileMission(mission, profile);
    const bool compiledOk = compiled.has_value();
    const bool uploaded = compiledOk && adapter.uploadMission(compiled.value());
    const auto readBack = adapter.readBackMission();
    const bool readBackOk = readBack.has_value();
    const auto comparison = readBackOk ? adapter.compareMission(compiled.value(), readBack.value()) : MissionComparison{};
    const bool verified = comparison.equal;

    const auto start = adapter.startMission();
    const bool execution = start.acknowledged && start.executionState == ExecutionState::Acknowledged;
    const auto flightState = adapter.getMissionState();
    const bool flightEvidence = flightState.value == "LOADED";

    const auto log = adapter.acquireFlightLog("LOG-027");
    const bool logAcquired = log.accepted && log.executionState == ExecutionState::Completed && log.sourceReference == "SIL_LOG_SOURCE:LOG-027";

    const auto close = adapter.close();
    const bool closed = close.acknowledged && close.executionState == ExecutionState::Completed && adapter.getConnectionState() == ConnectionState::Disconnected;

    std::cout << "PH4_INT_027|FLIGHT-RECORD-PH4-INT-027|UAV-E2E-027|MAVLINK2|BASELINE-1"
              << "|CAPABILITY=" << (capabilityGate ? "PASS" : "FAIL")
              << "|CONNECTED=" << (connected ? "PASS" : "FAIL")
              << "|IDENTIFIED=" << (identified ? "PASS" : "FAIL")
              << "|COMPILED=" << (compiledOk ? "PASS" : "FAIL")
              << "|UPLOADED=" << (uploaded ? "PASS" : "FAIL")
              << "|READ_BACK=" << (readBackOk ? "PASS" : "FAIL")
              << "|SEMANTIC_VERIFY=" << (verified ? "PASS" : "FAIL")
              << "|EXECUTION=" << (execution ? "PASS" : "FAIL")
              << "|FLIGHT_STATE=" << (flightEvidence ? "PASS" : "FAIL")
              << "|LOG=" << (logAcquired ? "PASS" : "FAIL")
              << "|CLOSED=" << (closed ? "PASS" : "FAIL")
              << std::endl;

    return capabilityGate && connected && identified && compiledOk && uploaded && readBackOk && verified && execution && flightEvidence && logAcquired && closed ? 0 : 1;
}
