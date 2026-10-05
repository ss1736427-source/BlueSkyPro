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
