#include "capability_execution_integration.hpp"

namespace bluesky::planning {

CapabilityExecutionMissionResult CapabilityExecutionIntegration::executeMission(
    operations::CapabilityAdmissionBoundary& admission_boundary,
    const std::string& device_id,
    const Mission& mission,
    const std::vector<std::string>& required_capabilities,
    const VehicleEquipmentCapabilityProfile& capabilities,
    bluesky::autopilot::UniversalAutopilotAdapter& adapter) {

    const auto admission = admission_boundary.admit({
        device_id,
        "mission:" + mission.identity.mission_id,
        required_capabilities
    });

    CapabilityExecutionMissionResult result;
    result.admission = admission;
    result.transfer.mission_id = mission.identity.mission_id;
    result.transfer.vehicle_id = capabilities.identity.vehicle_id;

    if (admission.state != operations::CapabilityAdmissionState::Admitted) {
        result.transfer.state = MissionTransferState::Rejected;
        result.transfer.reason = admission.reason;
        return result;
    }

    result.transfer = MissionTransferRuntime::execute(
        mission, capabilities, adapter);
    return result;
}

CapabilityExecutionCommandResult CapabilityExecutionIntegration::dispatchCommand(
    operations::CapabilityAdmissionBoundary& admission_boundary,
    const std::string& device_id,
    const command::CommandRequest& request,
    const std::vector<std::string>& required_capabilities,
    command::CommandLifecycleRuntime& lifecycle) {

    const auto admission = admission_boundary.admit({
        device_id,
        "command:" + request.command_id,
        required_capabilities
    });

    CapabilityExecutionCommandResult result;
    result.admission = admission;

    if (admission.state != operations::CapabilityAdmissionState::Admitted) {
        result.transition = {
            command::CommandState::Rejected,
            request.requested_at_ms,
            admission.reason
        };
        return result;
    }

    result.transition = lifecycle.validate(request);
    if (result.transition.state != command::CommandState::Validating) {
        return result;
    }

    result.transition = lifecycle.dispatch(request);
    return result;
}

} // namespace bluesky::planning
