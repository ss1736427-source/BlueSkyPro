#include "external_operational_request_intake.hpp"

#include <cassert>

using namespace bluesky::operations;

namespace {

OrchestratorPipelineContext executable_context() {
    OrchestratorPipelineContext context;
    context.runtime.runtime_state_version = "runtime-050";
    context.runtime.active_mission_package_version = "mission-050";
    context.runtime.actual_vehicle_state_version = "vehicle-050";
    context.runtime.mode = RuntimeCoordinationMode::Continue;
    context.runtime.runtime_state_current = true;
    context.runtime.active_package_consistent = true;
    context.runtime.safety_action_required = false;
    context.runtime.energy_gate_ok = true;
    context.runtime.c2_operational = true;
    context.policy_version = "policy-050";
    context.input_state_version = "state-050";
    context.selection.authority.system_authorized = true;
    return context;
}

ExternalOperationalRequest request(const std::string& action_id) {
    ExternalOperationalRequest value;
    value.request_id = "req-050";
    value.binding.action_id = action_id;
    value.binding.device_id = "dev-050";
    value.binding.kind = ActionExecutionKind::CommandDispatch;
    value.binding.required_capabilities = {"command.takeoff"};
    return value;
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

} // namespace

int main() {
    const auto accepted = ExternalOperationalRequestIntake::evaluate(
        request("action.takeoff"), snapshot(), executable_context());

    assert(accepted.state == ExternalOperationalRequestState::Accepted);
    assert(accepted.reason == "REQUEST_ACCEPTED");
    assert(accepted.pipeline.status == PipelineStatus::Selected);

    const auto mismatch = ExternalOperationalRequestIntake::evaluate(
        request("action.land"), snapshot(), executable_context());

    assert(mismatch.state == ExternalOperationalRequestState::Rejected);
    assert(mismatch.reason == "REQUESTED_ACTION_NOT_SELECTED");

    auto blocked_context = executable_context();
    blocked_context.runtime.energy_gate_ok = false;
    const auto blocked = ExternalOperationalRequestIntake::evaluate(
        request("action.takeoff"), snapshot(), blocked_context);

    assert(blocked.state == ExternalOperationalRequestState::Rejected);
    assert(blocked.reason == "ACTION_NOT_EXECUTABLE");

    auto invalid = request("action.takeoff");
    invalid.request_id.clear();
    const auto invalid_result = ExternalOperationalRequestIntake::evaluate(
        invalid, snapshot(), executable_context());

    assert(invalid_result.state == ExternalOperationalRequestState::Rejected);
    assert(invalid_result.reason == "REQUEST_ID_REQUIRED");

    return 0;
}
