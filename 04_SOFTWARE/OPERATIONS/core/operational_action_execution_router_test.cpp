#include "operational_action_execution_router.hpp"
#include "../../AUTOPILOT_ADAPTER/core/autopilot_adapter_baseline.hpp"

#include <cassert>

using namespace bluesky::operations;
using namespace bluesky::planning;

namespace {

OrchestratorPipelineResult selected(const std::string& action_id)
{
    OrchestratorPipelineResult result;
    result.status = PipelineStatus::Selected;
    static Action action;
    action = {};
    action.id = action_id;
    result.selection.status = SelectionStatus::Selected;
    result.selection.action = &action;
    result.selection.reason = SelectionReason::Executable;
    return result;
}

DeviceCapabilitySnapshot snapshot()
{
    DeviceCapabilitySnapshot value;
    value.device_id = "dev-049";
    value.vehicle_id = "veh-049";
    value.autopilot_id = "ap-049";
    value.firmware = "fw-1";
    value.protocol = "MAVLink";
    value.protocol_version = "2";
    value.lifecycle = DeviceCapabilityLifecycle::Validated;
    value.capabilities = {"command.takeoff", "mission.upload"};
    return value;
}

command::CommandRequest request()
{
    command::CommandRequest value;
    value.command_id = "cmd-049";
    value.mission_id = "mission-049";
    value.target_vehicle_id = "veh-049";
    value.command_class = "takeoff";
    value.authority_source = "test";
    value.requested_at_ms = 1000;
    value.correlation_id = "corr-049";
    return value;
}

} // namespace

int main()
{
    DeviceCapabilityRuntime runtime;
    assert(runtime.registerSnapshot(snapshot()) == DeviceCapabilityResult::Registered);
    CapabilityCompatibilityGate gate(runtime);
    CapabilityAdmissionBoundary admission(gate);

    command::CommandLifecycleRuntime lifecycle;
    const auto routed = OperationalActionExecutionRouter::routeCommand(
        selected("action.takeoff"),
        {"action.takeoff", "dev-049", ActionExecutionKind::CommandDispatch,
         {"command.takeoff"}},
        request(),
        lifecycle,
        admission);

    assert(routed.state == ActionExecutionRouteState::Routed);
    assert(routed.command.admission.state == CapabilityAdmissionState::Admitted);
    assert(routed.command.transition.state == command::CommandState::Dispatched);

    command::CommandLifecycleRuntime blocked;
    const auto mismatched = OperationalActionExecutionRouter::routeCommand(
        selected("action.other"),
        {"action.takeoff", "dev-049", ActionExecutionKind::CommandDispatch,
         {"command.takeoff"}},
        request(),
        blocked,
        admission);

    assert(mismatched.state == ActionExecutionRouteState::Rejected);
    assert(mismatched.reason == "ACTION_NOT_EXECUTABLE");
    assert(blocked.attemptCount() == 0);

    command::CommandLifecycleRuntime incompatible;
    const auto capability_block = OperationalActionExecutionRouter::routeCommand(
        selected("action.land"),
        {"action.land", "dev-049", ActionExecutionKind::CommandDispatch,
         {"command.land"}},
        request(),
        incompatible,
        admission);

    assert(capability_block.state == ActionExecutionRouteState::Routed);
    assert(capability_block.command.admission.state == CapabilityAdmissionState::Rejected);
    assert(incompatible.attemptCount() == 0);

    return 0;
}
