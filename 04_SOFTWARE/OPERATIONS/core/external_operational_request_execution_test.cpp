#include "external_operational_request_execution.hpp"
#include "../../AUTOPILOT_ADAPTER/core/autopilot_adapter_baseline.hpp"

#include <cassert>

using namespace bluesky::operations;
using namespace bluesky::planning;

namespace {

OrchestratorPipelineContext executable_context() {
    OrchestratorPipelineContext context;
    context.runtime.runtime_state_version = "runtime-051";
    context.runtime.active_mission_package_version = "mission-051";
    context.runtime.actual_vehicle_state_version = "vehicle-051";
    context.runtime.mode = RuntimeCoordinationMode::Continue;
    context.runtime.runtime_state_current = true;
    context.runtime.active_package_consistent = true;
    context.runtime.safety_action_required = false;
    context.runtime.energy_gate_ok = true;
    context.runtime.c2_operational = true;
    context.policy_version = "policy-051";
    context.input_state_version = "state-051";
    context.selection.authority.system_authorized = true;
    return context;
}

OperationalSnapshot snapshot() {
    OperationalSnapshot value;
    Action action;
    action.id = "action.takeoff";
    action.authorized = true;
    action.safe = true;
    action.regulatory_ok = true;
    action.energy_ok = true;
    action.priority = 10;
    value.actions.push_back(action);
    return value;
}

ExternalOperationalRequest request() {
    ExternalOperationalRequest value;
    value.request_id = "req-051";
    value.binding.action_id = "action.takeoff";
    value.binding.device_id = "dev-051";
    value.binding.kind = ActionExecutionKind::CommandDispatch;
    value.binding.required_capabilities = {"command.takeoff"};
    return value;
}

DeviceCapabilitySnapshot capability_snapshot() {
    DeviceCapabilitySnapshot value;
    value.device_id = "dev-051";
    value.vehicle_id = "veh-051";
    value.autopilot_id = "ap-051";
    value.firmware = "fw-1";
    value.protocol = "MAVLink";
    value.protocol_version = "2";
    value.lifecycle = DeviceCapabilityLifecycle::Validated;
    value.capabilities = {"command.takeoff"};
    return value;
}

command::CommandRequest command_request() {
    command::CommandRequest value;
    value.command_id = "cmd-051";
    value.mission_id = "mission-051";
    value.target_vehicle_id = "veh-051";
    value.command_class = "takeoff";
    value.authority_source = "test";
    value.requested_at_ms = 1000;
    value.correlation_id = "corr-051";
    return value;
}

} // namespace

int main() {
    DeviceCapabilityRuntime runtime;
    assert(runtime.registerSnapshot(capability_snapshot()) == DeviceCapabilityResult::Registered);
    CapabilityCompatibilityGate gate(runtime);
    CapabilityAdmissionBoundary admission(gate);

    const auto accepted_request = ExternalOperationalRequestIntake::evaluate(
        request(), snapshot(), executable_context());
    assert(accepted_request.state == ExternalOperationalRequestState::Accepted);

    command::CommandLifecycleRuntime lifecycle;
    const auto routed = ExternalOperationalRequestExecution::executeCommand(
        accepted_request, command_request(), lifecycle, admission);
    assert(routed.state == ActionExecutionRouteState::Routed);
    assert(routed.command.admission.state == CapabilityAdmissionState::Admitted);
    assert(routed.command.transition.state == command::CommandState::Dispatched);
    assert(lifecycle.attemptCount() == 1);

    auto rejected_context = executable_context();
    rejected_context.runtime.energy_gate_ok = false;
    const auto rejected_request = ExternalOperationalRequestIntake::evaluate(
        request(), snapshot(), rejected_context);
    assert(rejected_request.state == ExternalOperationalRequestState::Rejected);

    command::CommandLifecycleRuntime blocked;
    const auto not_executed = ExternalOperationalRequestExecution::executeCommand(
        rejected_request, command_request(), blocked, admission);
    assert(not_executed.state == ActionExecutionRouteState::Rejected);
    assert(not_executed.reason == "REQUEST_NOT_ACCEPTED");
    assert(blocked.attemptCount() == 0);

    auto mismatch_request = request();
    mismatch_request.binding.action_id = "action.land";
    const auto mismatch = ExternalOperationalRequestIntake::evaluate(
        mismatch_request, snapshot(), executable_context());
    assert(mismatch.state == ExternalOperationalRequestState::Rejected);

    command::CommandLifecycleRuntime mismatch_lifecycle;
    const auto mismatch_not_executed = ExternalOperationalRequestExecution::executeCommand(
        mismatch, command_request(), mismatch_lifecycle, admission);
    assert(mismatch_not_executed.state == ActionExecutionRouteState::Rejected);
    assert(mismatch_lifecycle.attemptCount() == 0);

    auto incompatible_request = request();
    incompatible_request.binding.required_capabilities = {"command.land"};
    const auto incompatible_intake = ExternalOperationalRequestIntake::evaluate(
        incompatible_request, snapshot(), executable_context());
    assert(incompatible_intake.state == ExternalOperationalRequestState::Accepted);

    command::CommandLifecycleRuntime incompatible_lifecycle;
    const auto capability_block = ExternalOperationalRequestExecution::executeCommand(
        incompatible_intake, command_request(), incompatible_lifecycle, admission);
    assert(capability_block.state == ActionExecutionRouteState::Routed);
    assert(capability_block.command.admission.state == CapabilityAdmissionState::Rejected);
    assert(incompatible_lifecycle.attemptCount() == 0);

    auto wrong_device = request();
    wrong_device.binding.device_id = "dev-other";
    const auto wrong_device_intake = ExternalOperationalRequestIntake::evaluate(
        wrong_device, snapshot(), executable_context());
    assert(wrong_device_intake.state == ExternalOperationalRequestState::Accepted);

    command::CommandLifecycleRuntime wrong_device_lifecycle;
    const auto wrong_device_result = ExternalOperationalRequestExecution::executeCommand(
        wrong_device_intake, command_request(), wrong_device_lifecycle, admission);
    assert(wrong_device_result.command.admission.state == CapabilityAdmissionState::Rejected);
    assert(wrong_device_lifecycle.attemptCount() == 0);

    return 0;
}
