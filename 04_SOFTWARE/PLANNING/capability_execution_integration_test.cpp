#include "capability_execution_integration.hpp"
#include "../AUTOPILOT_ADAPTER/core/autopilot_adapter_baseline.hpp"

#include <cassert>

using namespace bluesky::operations;
using namespace bluesky::planning;

namespace {

DeviceCapabilitySnapshot validated()
{
    DeviceCapabilitySnapshot snapshot;
    snapshot.device_id = "dev-048";
    snapshot.vehicle_id = "veh-048";
    snapshot.autopilot_id = "ap-048";
    snapshot.firmware = "fw-1";
    snapshot.protocol = "MAVLink";
    snapshot.protocol_version = "2";
    snapshot.lifecycle = DeviceCapabilityLifecycle::Validated;
    snapshot.capabilities = {"mission.upload", "command.takeoff", "telemetry"};
    return snapshot;
}

command::CommandRequest command_request()
{
    command::CommandRequest request;
    request.command_id = "cmd-048";
    request.mission_id = "mission-048";
    request.target_vehicle_id = "veh-048";
    request.command_class = "takeoff";
    request.authority_source = "test";
    request.requested_at_ms = 1000;
    request.correlation_id = "corr-048";
    return request;
}

void setReleasedLifecycle(Mission& mission)
{
    auto& lifecycle = mission.lifecycle;
    lifecycle.mission_id = mission.identity.mission_id;
    lifecycle.mission_version = mission.identity.mission_version;
    lifecycle.current_route_id = "route-048";
    lifecycle.current_route_version = "3";
    lifecycle.current_calculation_input_version = "input-9";
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
        event.actor_id = "operator-048";
        event.timestamp = "2026-11-15T10:00:00Z";
        event.from = from[i];
        event.to = to[i];
        event.route_id = lifecycle.current_route_id;
        event.route_version = lifecycle.current_route_version;
        event.evidence_reference = lifecycle.current_calculation_input_version;
        lifecycle.history.push_back(event);
    }
    lifecycle.next_event_sequence = 5;
}

} // namespace

int main()
{
    DeviceCapabilityRuntime runtime;
    assert(runtime.registerSnapshot(validated()) == DeviceCapabilityResult::Registered);

    CapabilityCompatibilityGate gate(runtime);
    CapabilityAdmissionBoundary admission(gate);

    command::CommandLifecycleRuntime lifecycle;
    const auto command_result = CapabilityExecutionIntegration::dispatchCommand(
        admission,
        "dev-048",
        command_request(),
        {"command.takeoff"},
        lifecycle);

    assert(command_result.admission.state == CapabilityAdmissionState::Admitted);
    assert(command_result.transition.state == command::CommandState::Dispatched);
    assert(lifecycle.attemptCount() == 1);

    command::CommandLifecycleRuntime blocked_lifecycle;
    const auto blocked_command = CapabilityExecutionIntegration::dispatchCommand(
        admission,
        "dev-048",
        command_request(),
        {"command.land"},
        blocked_lifecycle);

    assert(blocked_command.admission.state == CapabilityAdmissionState::Rejected);
    assert(blocked_command.admission.compatibility == CapabilityCompatibilityResult::MissingCapability);
    assert(blocked_command.transition.state == command::CommandState::Rejected);
    assert(blocked_lifecycle.attemptCount() == 0);

    Mission mission;
    mission.identity.mission_id = "mission-048";
    mission.identity.mission_version = "1";
    mission.vehicle_assignments.push_back({"veh-048", "cfg-048"});
    mission.route_candidates.push_back({"candidate-048", "solver-048", "1"});
    mission.selected_solution.candidate_id = "candidate-048";
    mission.selected_solution.calculation_input_version = "input-9";
    mission.selected_solution.algorithm_version = "algorithm-1";
    mission.selected_solution.model_version = "model-1";

    VehicleEquipmentCapabilityProfile profile;
    profile.identity.vehicle_id = "veh-048";
    profile.identity.autopilot_type = "test";
    profile.identity.supported_protocols = {"MAVLink"};
    profile.configuration.configuration_version = "cfg-048";
    profile.configuration.valid_for_mission = true;

    bluesky::autopilot::Identity adapter_identity;
    adapter_identity.vehicleId = "veh-048";
    adapter_identity.vehicleType = "test";
    adapter_identity.autopilot = "test";
    adapter_identity.firmwareVersion = "1";
    adapter_identity.protocol = "MAVLink";
    adapter_identity.protocolVersion = "2";
    bluesky::autopilot::AutopilotAdapterBaseline adapter(adapter_identity);
    const bool connected = adapter.connect();
    assert(connected);

    const auto unfinalized_mission = CapabilityExecutionIntegration::executeMission(
        admission, "dev-048", mission, {"mission.upload"}, profile, adapter);
    assert(unfinalized_mission.admission.state == CapabilityAdmissionState::Admitted);
    assert(unfinalized_mission.transfer.state == MissionTransferState::Rejected);
    assert(unfinalized_mission.transfer.reason == "MISSION_LIFECYCLE_RELEASE_EVENT_REQUIRED");

    // READY alone is insufficient: regulatory/authorization/release gates must
    // also have completed before the autopilot upload path is opened.
    mission.lifecycle.mission_id = mission.identity.mission_id;
    mission.lifecycle.mission_version = mission.identity.mission_version;
    mission.lifecycle.state = MissionLifecycleState::Ready;
    const auto ready_only_mission = CapabilityExecutionIntegration::executeMission(
        admission, "dev-048", mission, {"mission.upload"}, profile, adapter);
    assert(ready_only_mission.transfer.state == MissionTransferState::Rejected);
    assert(ready_only_mission.transfer.reason == "MISSION_LIFECYCLE_RELEASE_EVENT_REQUIRED");

    setReleasedLifecycle(mission);
    const auto admitted_mission = CapabilityExecutionIntegration::executeMission(
        admission, "dev-048", mission, {"mission.upload"}, profile, adapter);

    assert(admitted_mission.admission.state == CapabilityAdmissionState::Admitted);
    assert(admitted_mission.transfer.state == MissionTransferState::Verified);
    assert(admitted_mission.transfer.reason == "MISSION_UPLOADED_READBACK_VERIFIED");

    const auto rejected_mission = CapabilityExecutionIntegration::executeMission(
        admission, "dev-048", mission, {"mission.abort"}, profile, adapter);

    assert(rejected_mission.admission.state == CapabilityAdmissionState::Rejected);
    assert(rejected_mission.transfer.state == MissionTransferState::Rejected);
    assert(rejected_mission.transfer.reason == "REQUIRED_CAPABILITY_MISSING");

    // A lifecycle event from another route revision must never authorize upload.
    Mission stale_revision = mission;
    stale_revision.lifecycle.current_route_version = "4";
    const auto stale_route_mission = CapabilityExecutionIntegration::executeMission(
        admission, "dev-048", stale_revision, {"mission.upload"}, profile, adapter);
    assert(stale_route_mission.transfer.state == MissionTransferState::Rejected);
    assert(stale_route_mission.transfer.reason == "MISSION_LIFECYCLE_RELEASE_EVENT_REQUIRED");

    return 0;
}
