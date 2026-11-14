#include "capability_execution_integration.hpp"

namespace bluesky::planning {
namespace {

bool hasReleasedLifecycleForCurrentMission(const Mission& mission)
{
    const auto& lifecycle = mission.lifecycle;
    if (mission.identity.mission_id.empty()
        || mission.identity.mission_version.empty()
        || lifecycle.mission_id != mission.identity.mission_id
        || lifecycle.mission_version != mission.identity.mission_version
        || lifecycle.state != MissionLifecycleState::Released
        || lifecycle.current_route_id.empty()
        || lifecycle.current_route_version.empty()
        || lifecycle.current_calculation_input_version.empty()
        || lifecycle.history.empty()) {
        return false;
    }

    bool readySeen = false;
    bool releasedSeen = false;
    std::uint64_t previousSequence = 0;
    for (const auto& event : lifecycle.history) {
        if (event.sequence == 0 || event.sequence <= previousSequence
            || event.mission_id != mission.identity.mission_id
            || event.mission_version != mission.identity.mission_version
            || event.actor_id.empty() || event.timestamp.empty()
            || event.route_id != lifecycle.current_route_id
            || event.route_version != lifecycle.current_route_version
            || event.evidence_reference != lifecycle.current_calculation_input_version) {
            return false;
        }
        previousSequence = event.sequence;

        if (event.to == MissionLifecycleState::Ready
            && event.from == MissionLifecycleState::Validating) {
            readySeen = true;
        }
        if (event.to == MissionLifecycleState::Released
            && event.from == MissionLifecycleState::Authorized
            && readySeen) {
            releasedSeen = true;
        }
    }

    return releasedSeen
        && lifecycle.history.back().to == MissionLifecycleState::Released
        && lifecycle.history.back().route_id == lifecycle.current_route_id
        && lifecycle.history.back().route_version == lifecycle.current_route_version;
}

} // namespace

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

    // Upload is a release action, not a way to make a calculated mission
    // executable. Require the authoritative C++ lifecycle history to contain
    // READY followed by RELEASED for this exact mission/route/input revision.
    if (!hasReleasedLifecycleForCurrentMission(mission)) {
        result.transfer.state = MissionTransferState::Rejected;
        result.transfer.reason = "MISSION_LIFECYCLE_RELEASE_EVENT_REQUIRED";
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
