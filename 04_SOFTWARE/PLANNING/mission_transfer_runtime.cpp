#include "mission_transfer_runtime.hpp"

#include <algorithm>

namespace bluesky::planning {

namespace {

bool has_value(const std::string& value) {
    return !value.empty();
}

bool vehicle_assignment_matches(
    const Mission& mission,
    const VehicleEquipmentCapabilityProfile& capabilities) {
    if (mission.vehicle_assignments.size() != 1) return false;
    const auto& assignment = mission.vehicle_assignments.front();
    return assignment.vehicle_id == capabilities.identity.vehicle_id &&
           assignment.configuration_version == capabilities.configuration.configuration_version;
}

bool capability_valid(const VehicleEquipmentCapabilityProfile& capabilities) {
    return has_value(capabilities.identity.vehicle_id) &&
           has_value(capabilities.identity.autopilot_type) &&
           has_value(capabilities.configuration.configuration_version) &&
           capabilities.configuration.valid_for_mission;
}

// Validate the release chain at the lowest public transfer boundary. A caller
// must not be able to bypass lifecycle controls by invoking this runtime
// without going through CapabilityExecutionIntegration.
bool released_for_current_revision(const Mission& mission) {
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

    const MissionLifecycleState expectedFrom[] = {
        MissionLifecycleState::Validating,
        MissionLifecycleState::Ready,
        MissionLifecycleState::Submitted,
        MissionLifecycleState::Authorized
    };
    const MissionLifecycleState expectedTo[] = {
        MissionLifecycleState::Ready,
        MissionLifecycleState::Submitted,
        MissionLifecycleState::Authorized,
        MissionLifecycleState::Released
    };

    std::uint64_t previousSequence = 0;
    std::size_t releaseStage = 0;
    for (const auto& event : lifecycle.history) {
        if (event.sequence == 0 || event.sequence <= previousSequence
            || event.mission_id != mission.identity.mission_id
            || event.mission_version != mission.identity.mission_version
            || event.actor_id.empty() || event.timestamp.empty()) {
            return false;
        }
        previousSequence = event.sequence;

        if (releaseStage < 4
            && event.from == expectedFrom[releaseStage]
            && event.to == expectedTo[releaseStage]) {
            if (event.route_id != lifecycle.current_route_id
                || event.route_version != lifecycle.current_route_version
                || event.evidence_reference != lifecycle.current_calculation_input_version) {
                return false;
            }
            ++releaseStage;
        } else if (releaseStage > 0 && releaseStage < 4
                   && event.to == expectedTo[releaseStage - 1]) {
            // Duplicate or out-of-order lifecycle transitions invalidate the chain.
            return false;
        }
    }

    return releaseStage == 4
        && lifecycle.next_event_sequence > previousSequence
        && lifecycle.history.back().to == MissionLifecycleState::Released
        && lifecycle.history.back().from == MissionLifecycleState::Authorized
        && lifecycle.history.back().route_id == lifecycle.current_route_id
        && lifecycle.history.back().route_version == lifecycle.current_route_version
        && lifecycle.history.back().evidence_reference
            == lifecycle.current_calculation_input_version;
}

} // namespace

MissionTransferResult MissionTransferRuntime::execute(
    const Mission& mission,
    const VehicleEquipmentCapabilityProfile& capabilities,
    bluesky::autopilot::UniversalAutopilotAdapter& adapter) {

    MissionTransferResult result;
    result.mission_id = mission.identity.mission_id;
    result.vehicle_id = capabilities.identity.vehicle_id;

    if (!has_value(mission.identity.mission_id) ||
        !has_value(mission.identity.mission_version)) {
        result.reason = "MISSION_ID_AND_VERSION_REQUIRED";
        return result;
    }

    if (!capability_valid(capabilities)) {
        result.reason = "VEHICLE_CAPABILITY_PROFILE_INVALID";
        return result;
    }

    if (!vehicle_assignment_matches(mission, capabilities)) {
        result.reason = "MISSION_VEHICLE_CONFIGURATION_MISMATCH";
        return result;
    }

    // No compilation, connection, or upload is attempted until the C++
    // lifecycle history proves READY -> SUBMITTED -> AUTHORIZED -> RELEASED
    // for the exact current mission, route, and calculation-input revision.
    if (!released_for_current_revision(mission)) {
        result.reason = "MISSION_LIFECYCLE_RELEASE_EVENT_REQUIRED";
        return result;
    }

    const auto compatibility = adapter.checkCompatibility({
        capabilities.identity.supported_protocols.empty()
            ? ""
            : capabilities.identity.supported_protocols.front(),
        adapter.getProtocolVersion(),
        "mission_upload"
    });
    if (!compatibility.compatible) {
        result.reason = compatibility.reason;
        return result;
    }

    const auto compiled = adapter.compileMission(mission, capabilities);
    if (!compiled.has_value() || compiled->empty()) {
        result.state = MissionTransferState::Failed;
        result.reason = "MISSION_COMPILATION_FAILED";
        return result;
    }
    result.compiled_mission = *compiled;
    result.state = MissionTransferState::Compiled;

    if (adapter.getConnectionState() != bluesky::autopilot::ConnectionState::Connected) {
        result.state = MissionTransferState::Failed;
        result.reason = "AUTOPILOT_NOT_CONNECTED";
        return result;
    }

    if (!adapter.uploadMission(result.compiled_mission)) {
        result.state = MissionTransferState::Failed;
        result.reason = "MISSION_UPLOAD_FAILED";
        return result;
    }
    result.state = MissionTransferState::Uploaded;

    const auto readback = adapter.readBackMission();
    if (!readback.has_value() || readback->empty()) {
        result.state = MissionTransferState::Failed;
        result.reason = "MISSION_READBACK_FAILED";
        return result;
    }
    result.readback_mission = *readback;
    result.state = MissionTransferState::ReadBack;

    const auto comparison =
        adapter.compareMission(result.compiled_mission, result.readback_mission);
    if (!comparison.equal) {
        result.state = MissionTransferState::Failed;
        result.reason = comparison.mismatches.empty()
            ? "MISSION_SEMANTIC_VERIFICATION_FAILED"
            : comparison.mismatches.front();
        return result;
    }

    result.state = MissionTransferState::Verified;
    result.reason = "MISSION_UPLOADED_READBACK_VERIFIED";
    return result;
}

} // namespace bluesky::planning
