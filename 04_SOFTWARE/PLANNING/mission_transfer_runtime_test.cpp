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
    mission.selected_solution.calculation_input_version = "INPUT-7";
    return mission;
}

void setReleasedLifecycle(bluesky::planning::Mission& mission) {
    using namespace bluesky::planning;
    auto& lifecycle = mission.lifecycle;
    lifecycle.mission_id = mission.identity.mission_id;
    lifecycle.mission_version = mission.identity.mission_version;
    lifecycle.current_route_id = "ROUTE-1";
    lifecycle.current_route_version = "4";
    lifecycle.current_calculation_input_version = "INPUT-7";
    lifecycle.state = MissionLifecycleState::Released;

    const MissionLifecycleState from[] = {
        MissionLifecycleState::Validating,
        MissionLifecycleState::Ready,
        MissionLifecycleState::Submitted,
        MissionLifecycleState::Authorized
    };
    const MissionLifecycleState to[] = {
        MissionLifecycleState::Ready,
        MissionLifecycleState::Submitted,
        MissionLifecycleState::Authorized,
        MissionLifecycleState::Released
    };
    for (std::size_t i = 0; i < 4; ++i) {
        MissionLifecycleEvent event;
        event.sequence = i + 1;
        event.mission_id = mission.identity.mission_id;
        event.mission_version = mission.identity.mission_version;
        event.actor_id = "operator-020";
        event.timestamp = "2026-11-15T00:00:00Z";
        event.from = from[i];
        event.to = to[i];
        event.route_id = lifecycle.current_route_id;
        event.route_version = lifecycle.current_route_version;
        event.evidence_reference = lifecycle.current_calculation_input_version;
        lifecycle.history.push_back(event);
    }
    lifecycle.next_event_sequence = 5;
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

void testRejectsWithoutReleasedLifecycle() {
    auto mission = makeMission();
    auto capabilities = makeCapabilities();
    auto adapter = makeAdapter();
    assert(adapter.connect());

    const auto result =
        bluesky::planning::MissionTransferRuntime::execute(mission, capabilities, adapter);

    assert(result.state == bluesky::planning::MissionTransferState::Rejected);
    assert(result.reason == "MISSION_LIFECYCLE_RELEASE_EVENT_REQUIRED");
    assert(result.compiled_mission.empty());
}

void testRejectsReadyOnlyLifecycle() {
    auto mission = makeMission();
    mission.lifecycle.mission_id = mission.identity.mission_id;
    mission.lifecycle.mission_version = mission.identity.mission_version;
    mission.lifecycle.state = bluesky::planning::MissionLifecycleState::Ready;
    auto capabilities = makeCapabilities();
    auto adapter = makeAdapter();
    assert(adapter.connect());

    const auto result =
        bluesky::planning::MissionTransferRuntime::execute(mission, capabilities, adapter);

    assert(result.state == bluesky::planning::MissionTransferState::Rejected);
    assert(result.reason == "MISSION_LIFECYCLE_RELEASE_EVENT_REQUIRED");
}

void testFullTransfer() {
    auto mission = makeMission();
    setReleasedLifecycle(mission);
    auto capabilities = makeCapabilities();
    auto adapter = makeAdapter();
    assert(adapter.connect());

    const auto result =
        bluesky::planning::MissionTransferRuntime::execute(mission, capabilities, adapter);

    assert(result.state == bluesky::planning::MissionTransferState::Verified);
    assert(result.compiled_mission == result.readback_mission);
    assert(result.reason == "MISSION_UPLOADED_READBACK_VERIFIED");
}

void testRejectsStaleCalculationInputRevision() {
    auto mission = makeMission();
    setReleasedLifecycle(mission);
    mission.selected_solution.calculation_input_version = "INPUT-STALE";
    auto capabilities = makeCapabilities();
    auto adapter = makeAdapter();
    assert(adapter.connect());
    const auto result = bluesky::planning::MissionTransferRuntime::execute(mission, capabilities, adapter);
    assert(result.state == bluesky::planning::MissionTransferState::Rejected);
    assert(result.reason == "MISSION_LIFECYCLE_RELEASE_EVENT_REQUIRED");
}

void testRejectsInterleavedEventAfterReady() {
    auto mission = makeMission();
    setReleasedLifecycle(mission);
    auto unrelated = mission.lifecycle.history[1];
    unrelated.sequence = 3;
    unrelated.from = bluesky::planning::MissionLifecycleState::Submitted;
    unrelated.to = bluesky::planning::MissionLifecycleState::Paused;
    mission.lifecycle.history.insert(mission.lifecycle.history.begin() + 2, unrelated);
    for (std::size_t i = 3; i < mission.lifecycle.history.size(); ++i)
        mission.lifecycle.history[i].sequence = i + 1;
    mission.lifecycle.next_event_sequence = mission.lifecycle.history.back().sequence + 1;
    auto capabilities = makeCapabilities();
    auto adapter = makeAdapter();
    assert(adapter.connect());
    const auto result = bluesky::planning::MissionTransferRuntime::execute(mission, capabilities, adapter);
    assert(result.state == bluesky::planning::MissionTransferState::Rejected);
    assert(result.reason == "MISSION_LIFECYCLE_RELEASE_EVENT_REQUIRED");
}

void testRejectsSkippedReleaseTransition() {
    auto mission = makeMission();
    setReleasedLifecycle(mission);
    mission.lifecycle.history[2].from = bluesky::planning::MissionLifecycleState::Submitted;
    mission.lifecycle.history[2].to = bluesky::planning::MissionLifecycleState::Released;
    mission.lifecycle.history.erase(mission.lifecycle.history.begin() + 3);
    mission.lifecycle.next_event_sequence = 4;
    auto capabilities = makeCapabilities();
    auto adapter = makeAdapter();
    assert(adapter.connect());
    const auto result = bluesky::planning::MissionTransferRuntime::execute(mission, capabilities, adapter);
    assert(result.state == bluesky::planning::MissionTransferState::Rejected);
    assert(result.reason == "MISSION_LIFECYCLE_RELEASE_EVENT_REQUIRED");
}

void testRejectsStaleRouteRevision() {
    auto mission = makeMission();
    setReleasedLifecycle(mission);
    mission.lifecycle.current_route_version = "5";
    auto capabilities = makeCapabilities();
    auto adapter = makeAdapter();
    assert(adapter.connect());

    const auto result =
        bluesky::planning::MissionTransferRuntime::execute(mission, capabilities, adapter);

    assert(result.state == bluesky::planning::MissionTransferState::Rejected);
    assert(result.reason == "MISSION_LIFECYCLE_RELEASE_EVENT_REQUIRED");
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
    setReleasedLifecycle(mission);
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
    setReleasedLifecycle(mission);
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
    testRejectsWithoutReleasedLifecycle();
    testRejectsReadyOnlyLifecycle();
    testFullTransfer();
    testRejectsStaleCalculationInputRevision();
    testRejectsInterleavedEventAfterReady();
    testRejectsSkippedReleaseTransition();
    testRejectsStaleRouteRevision();
    testRejectsConfigurationMismatch();
    testRejectsInvalidCapabilities();
    testRejectsWhenDisconnected();
    testRejectsReadbackMismatch();
    std::cout << "mission_transfer_runtime_test: PASS\n";
    return 0;
}
