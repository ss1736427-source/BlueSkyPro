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

    VehicleEquipmentCapabilityProfile profile;
    profile.identity.vehicle_id = "veh-048";
    profile.identity.autopilot_type = "test";
    profile.identity.supported_protocols = {"MAVLink"};
    profile.configuration.configuration_version = "cfg-048";
    profile.configuration.valid_for_mission = true;

    bluesky::autopilot::AutopilotAdapterBaseline adapter;
    const auto admitted_mission = CapabilityExecutionIntegration::executeMission(
        admission,
        "dev-048",
        mission,
        {"mission.upload"},
        profile,
        adapter);

    assert(admitted_mission.admission.state == CapabilityAdmissionState::Admitted);
    assert(admitted_mission.transfer.state != MissionTransferState::Rejected);

    const auto rejected_mission = CapabilityExecutionIntegration::executeMission(
        admission,
        "dev-048",
        mission,
        {"mission.abort"},
        profile,
        adapter);

    assert(rejected_mission.admission.state == CapabilityAdmissionState::Rejected);
    assert(rejected_mission.transfer.state == MissionTransferState::Rejected);
    assert(rejected_mission.transfer.reason == "REQUIRED_CAPABILITY_MISSING");

    return 0;
}
