#include "mission_transfer_runtime.hpp"
#include "../AUTOPILOT_ADAPTER/core/autopilot_adapter_baseline.hpp"

#include <cassert>
#include <iostream>
#include <optional>

namespace {

bluesky::planning::Mission makeMission() {
    bluesky::planning::Mission mission;
    mission.identity.mission_id = "MISSION-020";
    mission.identity.mission_version = "1";
    mission.vehicle_assignments.push_back({"UAV-020", "CFG-1"});
    mission.route_candidates.push_back({"ROUTE-1", "SOLVER", "1"});
    mission.selected_solution.candidate_id = "ROUTE-1";
    mission.selected_solution.algorithm_version = "ALG-1";
    mission.selected_solution.model_version = "MODEL-1";
    return mission;
}

bluesky::planning::VehicleEquipmentCapabilityProfile makeCapabilities() {
    bluesky::planning::VehicleEquipmentCapabilityProfile capabilities;
    capabilities.identity.vehicle_id = "UAV-020";
    capabilities.identity.autopilot_type = "TEST-FCS";
    capabilities.identity.autopilot_version = "1";
    capabilities.identity.supported_protocols = {"MAVLink2"};
    capabilities.configuration.configuration_version = "CFG-1";
    capabilities.configuration.valid_for_mission = true;
    return capabilities;
}

bluesky::autopilot::AutopilotAdapterBaseline makeAdapter() {
    bluesky::autopilot::Identity identity;
    identity.vehicleId = "UAV-020";
    identity.vehicleType = "TEST";
    identity.autopilot = "TEST-FCS";
    identity.firmwareVersion = "1";
    identity.protocol = "MAVLink2";
    identity.protocolVersion = "2";
    return bluesky::autopilot::AutopilotAdapterBaseline(identity);
}

void testFullTransfer() {
    auto mission = makeMission();
    auto capabilities = makeCapabilities();
    auto adapter = makeAdapter();
    assert(adapter.connect());

    const auto result =
        bluesky::planning::MissionTransferRuntime::execute(mission, capabilities, adapter);

    assert(result.state == bluesky::planning::MissionTransferState::Verified);
    assert(result.compiled_mission == result.readback_mission);
    assert(result.reason == "MISSION_UPLOADED_READBACK_VERIFIED");
}

void testRejectsConfigurationMismatch() {
    auto mission = makeMission();
    auto capabilities = makeCapabilities();
    capabilities.configuration.configuration_version = "CFG-2";
    auto adapter = makeAdapter();

    const auto result =
        bluesky::planning::MissionTransferRuntime::execute(mission, capabilities, adapter);

    assert(result.state == bluesky::planning::MissionTransferState::Rejected);
    assert(result.reason == "MISSION_VEHICLE_CONFIGURATION_MISMATCH");
}

void testRejectsInvalidCapabilities() {
    auto mission = makeMission();
    auto capabilities = makeCapabilities();
    capabilities.configuration.valid_for_mission = false;
    auto adapter = makeAdapter();

    const auto result =
        bluesky::planning::MissionTransferRuntime::execute(mission, capabilities, adapter);

    assert(result.state == bluesky::planning::MissionTransferState::Rejected);
    assert(result.reason == "VEHICLE_CAPABILITY_PROFILE_INVALID");
}

void testRejectsWhenDisconnected() {
    auto mission = makeMission();
    auto capabilities = makeCapabilities();
    auto adapter = makeAdapter();

    const auto result =
        bluesky::planning::MissionTransferRuntime::execute(mission, capabilities, adapter);

    assert(result.state == bluesky::planning::MissionTransferState::Failed);
    assert(result.reason == "AUTOPILOT_NOT_CONNECTED");
}

void testRejectsReadbackMismatch() {
    class MismatchAdapter final : public bluesky::autopilot::AutopilotAdapterBaseline {
    public:
        using AutopilotAdapterBaseline::AutopilotAdapterBaseline;
        std::optional<std::string> readBackMission() override {
            return std::string("DIFFERENT_MISSION");
        }
    };

    bluesky::autopilot::Identity identity;
    identity.vehicleId = "UAV-020";
    identity.autopilot = "TEST-FCS";
    identity.protocol = "MAVLink2";
    identity.protocolVersion = "2";

    auto mission = makeMission();
    auto capabilities = makeCapabilities();
    MismatchAdapter adapter(identity);
    assert(adapter.connect());

    const auto result =
        bluesky::planning::MissionTransferRuntime::execute(mission, capabilities, adapter);

    assert(result.state == bluesky::planning::MissionTransferState::Failed);
    assert(result.reason == "MISSION_CONTENT_MISMATCH");
}

} // namespace

int main() {
    testFullTransfer();
    testRejectsConfigurationMismatch();
    testRejectsInvalidCapabilities();
    testRejectsWhenDisconnected();
    testRejectsReadbackMismatch();
    std::cout << "mission_transfer_runtime_test: PASS\n";
    return 0;
}
