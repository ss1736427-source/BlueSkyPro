#include "operational_action_execution_router.hpp"

namespace bluesky::operations {

namespace {

bool decision_allows_execution(
    const OrchestratorPipelineResult& decision,
    const ActionExecutionBinding& binding) {
    return decision.status == PipelineStatus::Selected &&
           decision.selection.action != nullptr &&
           decision.selection.action->id == binding.action_id &&
           decision.selection.reason == SelectionReason::Executable;
}

ActionExecutionRouteResult rejected(
    const ActionExecutionBinding& binding,
    const std::string& reason) {
    ActionExecutionRouteResult result;
    result.action_id = binding.action_id;
    result.reason = reason;
    return result;
}

} // namespace

ActionExecutionRouteResult OperationalActionExecutionRouter::routeMission(
    const OrchestratorPipelineResult& decision,
    const ActionExecutionBinding& binding,
    const bluesky::planning::Mission& mission,
    const bluesky::planning::VehicleEquipmentCapabilityProfile& capabilities,
    bluesky::autopilot::UniversalAutopilotAdapter& adapter,
    CapabilityAdmissionBoundary& admission_boundary) {

    if (binding.kind != ActionExecutionKind::MissionTransfer) {
        return rejected(binding, "ACTION_EXECUTION_KIND_MISMATCH");
    }

    if (!decision_allows_execution(decision, binding)) {
        return rejected(binding, "ACTION_NOT_EXECUTABLE");
    }

    ActionExecutionRouteResult result;
    result.state = ActionExecutionRouteState::Routed;
    result.action_id = binding.action_id;
    result.mission = bluesky::planning::CapabilityExecutionIntegration::executeMission(
        admission_boundary,
        binding.device_id,
        mission,
        binding.required_capabilities,
        capabilities,
        adapter);
    result.reason = result.mission.admission.state == CapabilityAdmissionState::Admitted
        ? "MISSION_ROUTED"
        : result.mission.admission.reason;
    return result;
}

ActionExecutionRouteResult OperationalActionExecutionRouter::routeCommand(
    const OrchestratorPipelineResult& decision,
    const ActionExecutionBinding& binding,
    const bluesky::planning::command::CommandRequest& request,
    bluesky::planning::command::CommandLifecycleRuntime& lifecycle,
    CapabilityAdmissionBoundary& admission_boundary) {

    if (binding.kind != ActionExecutionKind::CommandDispatch) {
        return rejected(binding, "ACTION_EXECUTION_KIND_MISMATCH");
    }

    if (!decision_allows_execution(decision, binding)) {
        return rejected(binding, "ACTION_NOT_EXECUTABLE");
    }

    ActionExecutionRouteResult result;
    result.state = ActionExecutionRouteState::Routed;
    result.action_id = binding.action_id;
    result.command = bluesky::planning::CapabilityExecutionIntegration::dispatchCommand(
        admission_boundary,
        binding.device_id,
        request,
        binding.required_capabilities,
        lifecycle);
    result.reason = result.command.admission.state == CapabilityAdmissionState::Admitted
        ? "COMMAND_ROUTED"
        : result.command.admission.reason;
    return result;
}

} // namespace bluesky::operations
