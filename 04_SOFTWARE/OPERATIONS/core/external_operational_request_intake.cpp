#include "external_operational_request_intake.hpp"

namespace bluesky::operations {

ExternalOperationalRequestResult ExternalOperationalRequestIntake::evaluate(
    const ExternalOperationalRequest& request,
    const OperationalSnapshot& snapshot,
    const OrchestratorPipelineContext& context) {

    ExternalOperationalRequestResult result;
    result.request_id = request.request_id;
    result.action_id = request.binding.action_id;
    result.binding = request.binding;

    if (request.request_id.empty()) {
        result.reason = "REQUEST_ID_REQUIRED";
        return result;
    }

    if (request.binding.action_id.empty()) {
        result.reason = "ACTION_ID_REQUIRED";
        return result;
    }

    if (request.binding.device_id.empty()) {
        result.reason = "DEVICE_ID_REQUIRED";
        return result;
    }

    if (request.binding.required_capabilities.empty()) {
        result.reason = "REQUIRED_CAPABILITIES_REQUIRED";
        return result;
    }

    result.pipeline = run_orchestrator_pipeline(snapshot, context);

    if (result.pipeline.status != PipelineStatus::Selected ||
        result.pipeline.selection.action == nullptr ||
        result.pipeline.selection.reason != SelectionReason::Executable) {
        result.reason = "ACTION_NOT_EXECUTABLE";
        return result;
    }

    if (result.pipeline.selection.action->id != request.binding.action_id) {
        result.reason = "REQUESTED_ACTION_NOT_SELECTED";
        return result;
    }

    result.state = ExternalOperationalRequestState::Accepted;
    result.reason = "REQUEST_ACCEPTED";
    return result;
}

} // namespace bluesky::operations
