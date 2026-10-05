#include "external_operational_request_execution.hpp"

namespace bluesky::operations {

namespace {

ActionExecutionRouteResult rejected(const ExternalOperationalRequestResult& request, const std::string& reason) {
    ActionExecutionRouteResult result;
    result.action_id = request.action_id;
    result.reason = reason;
    return result;
}

bool accepted(const ExternalOperationalRequestResult& request) {
    return request.state == ExternalOperationalRequestState::Accepted &&
           request.pipeline.status == PipelineStatus::Selected &&
           request.pipeline.selection.action != nullptr &&
           request.pipeline.selection.action->id == request.binding.action_id &&
           request.pipeline.selection.reason == SelectionReason::Executable;
}

} // namespace

ActionExecutionRouteResult ExternalOperationalRequestExecution::executeMission(
    const ExternalOperationalRequestResult& accepted_request,
    const bluesky::planning::Mission& mission,
    const bluesky::planning::VehicleEquipmentCapabilityProfile& capabilities,
    bluesky::autopilot::UniversalAutopilotAdapter& adapter,
    CapabilityAdmissionBoundary& admission_boundary) {

    if (!accepted(accepted_request)) {
        return rejected(accepted_request, "REQUEST_NOT_ACCEPTED");
    }

    return OperationalActionExecutionRouter::routeMission(
        accepted_request.pipeline,
        accepted_request.binding,
        mission,
        capabilities,
        adapter,
        admission_boundary);
}

ActionExecutionRouteResult ExternalOperationalRequestExecution::executeCommand(
    const ExternalOperationalRequestResult& accepted_request,
    const bluesky::planning::command::CommandRequest& request,
    bluesky::planning::command::CommandLifecycleRuntime& lifecycle,
    CapabilityAdmissionBoundary& admission_boundary) {

    if (!accepted(accepted_request)) {
        return rejected(accepted_request, "REQUEST_NOT_ACCEPTED");
    }

    return OperationalActionExecutionRouter::routeCommand(
        accepted_request.pipeline,
        accepted_request.binding,
        request,
        lifecycle,
        admission_boundary);
}

} // namespace bluesky::operations
