#include "mission_lifecycle.hpp"

#include <algorithm>
#include <utility>

namespace bluesky::planning {
namespace {

bool isTerminal(MissionLifecycleState state) noexcept {
    return state == MissionLifecycleState::Completed || state == MissionLifecycleState::Aborted;
}

bool isKnownState(MissionLifecycleState state) noexcept {
    switch (state) {
    case MissionLifecycleState::Draft:
    case MissionLifecycleState::Calculating:
    case MissionLifecycleState::Validating:
    case MissionLifecycleState::Ready:
    case MissionLifecycleState::Submitted:
    case MissionLifecycleState::Authorized:
    case MissionLifecycleState::Released:
    case MissionLifecycleState::Executing:
    case MissionLifecycleState::Paused:
    case MissionLifecycleState::Replanning:
    case MissionLifecycleState::Completed:
    case MissionLifecycleState::Aborted:
        return true;
    }
    return false;
}

bool allowed(MissionLifecycleState from, MissionLifecycleState to) noexcept {
    if (to == MissionLifecycleState::Aborted && !isTerminal(from))
        return true;
    switch (from) {
    case MissionLifecycleState::Draft: return to == MissionLifecycleState::Calculating;
    case MissionLifecycleState::Calculating: return to == MissionLifecycleState::Validating;
    case MissionLifecycleState::Validating: return to == MissionLifecycleState::Ready;
    case MissionLifecycleState::Ready: return to == MissionLifecycleState::Submitted;
    case MissionLifecycleState::Submitted: return to == MissionLifecycleState::Authorized;
    case MissionLifecycleState::Authorized: return to == MissionLifecycleState::Released;
    case MissionLifecycleState::Released: return to == MissionLifecycleState::Executing;
    case MissionLifecycleState::Executing:
        return to == MissionLifecycleState::Paused || to == MissionLifecycleState::Completed;
    case MissionLifecycleState::Paused:
        return to == MissionLifecycleState::Replanning;
    case MissionLifecycleState::Replanning:
        return to == MissionLifecycleState::Calculating;
    case MissionLifecycleState::Completed:
    case MissionLifecycleState::Aborted:
        return false;
    }
    return false;
}

void require(bool condition, const char *code, std::vector<std::string> &blockers) {
    if (!condition)
        blockers.emplace_back(code);
}

void requireText(const std::string &value, const char *code,
                 std::vector<std::string> &blockers) {
    require(!value.empty(), code, blockers);
}

} // namespace

const char *toString(MissionLifecycleState state) noexcept {
    switch (state) {
    case MissionLifecycleState::Draft: return "DRAFT";
    case MissionLifecycleState::Calculating: return "CALCULATING";
    case MissionLifecycleState::Validating: return "VALIDATING";
    case MissionLifecycleState::Ready: return "READY";
    case MissionLifecycleState::Submitted: return "SUBMITTED";
    case MissionLifecycleState::Authorized: return "AUTHORIZED";
    case MissionLifecycleState::Released: return "RELEASED";
    case MissionLifecycleState::Executing: return "EXECUTING";
    case MissionLifecycleState::Paused: return "PAUSED";
    case MissionLifecycleState::Replanning: return "REPLANNING";
    case MissionLifecycleState::Completed: return "COMPLETED";
    case MissionLifecycleState::Aborted: return "ABORTED";
    }
    return "UNKNOWN";
}

MissionLifecycleTransitionResult MissionLifecycleController::transition(
    MissionLifecycleRecord &record,
    MissionLifecycleState target,
    const MissionLifecycleEvidence &evidence,
    const std::string &actor_id,
    const std::string &timestamp,
    const std::string &reason) {

    MissionLifecycleTransitionResult result;
    result.state = record.state;

    if (!isKnownState(record.state) || !isKnownState(target)) {
        result.blockers.emplace_back("MISSION_LIFECYCLE_STATE_UNKNOWN");
        return result;
    }
    if (record.mission_id.empty() || record.mission_version.empty()) {
        result.blockers.emplace_back("MISSION_IDENTITY_REQUIRED");
        return result;
    }
    const bool startsNewRevision = record.state == MissionLifecycleState::Replanning
        && target == MissionLifecycleState::Calculating;
    if (evidence.mission_id != record.mission_id
        || (startsNewRevision
            ? (evidence.mission_version.empty() || evidence.mission_version == record.mission_version)
            : evidence.mission_version != record.mission_version)) {
        result.blockers.emplace_back("LIFECYCLE_EVIDENCE_MISSION_LINEAGE_MISMATCH");
        return result;
    }
    if (actor_id.empty())
        result.blockers.emplace_back("LIFECYCLE_ACTOR_REQUIRED");
    if (timestamp.empty())
        result.blockers.emplace_back("LIFECYCLE_TIMESTAMP_REQUIRED");
    if (!allowed(record.state, target))
        result.blockers.emplace_back("MISSION_LIFECYCLE_TRANSITION_NOT_ALLOWED");

    if (target == MissionLifecycleState::Aborted) {
        requireText(reason, "ABORT_REASON_REQUIRED", result.blockers);
    } else if (record.state == MissionLifecycleState::Paused
               && target == MissionLifecycleState::Replanning) {
        requireText(reason, "REPLANNING_REASON_REQUIRED", result.blockers);
    } else if (record.state == MissionLifecycleState::Executing
               && target == MissionLifecycleState::Paused) {
        requireText(reason, "PAUSE_REASON_REQUIRED", result.blockers);
    }

    switch (target) {
    case MissionLifecycleState::Calculating:
        if (record.state == MissionLifecycleState::Draft) {
            require(evidence.task_definition_complete, "TASK_DEFINITION_INCOMPLETE", result.blockers);
            require(evidence.mission_geometry_complete, "MISSION_GEOMETRY_INCOMPLETE", result.blockers);
            require(evidence.vehicle_configuration_validated, "VEHICLE_CONFIGURATION_NOT_VALIDATED", result.blockers);
            require(evidence.equipment_configuration_validated, "EQUIPMENT_CONFIGURATION_NOT_VALIDATED", result.blockers);
            require(evidence.mission_notam_snapshot_current, "MISSION_NOTAM_SNAPSHOT_MISSING_OR_STALE", result.blockers);
            requireText(evidence.mission_notam_snapshot_id, "MISSION_NOTAM_SNAPSHOT_ID_REQUIRED", result.blockers);
            require(evidence.terrain_snapshot_current, "TERRAIN_SNAPSHOT_MISSING_OR_STALE", result.blockers);
            requireText(evidence.terrain_snapshot_id, "TERRAIN_SNAPSHOT_ID_REQUIRED", result.blockers);
        } else if (record.state == MissionLifecycleState::Replanning) {
            requireText(evidence.route_id, "REPLAN_ROUTE_ID_REQUIRED", result.blockers);
            requireText(evidence.route_version, "REPLAN_PARENT_ROUTE_VERSION_REQUIRED", result.blockers);
            requireText(evidence.calculation_input_version, "REPLAN_INPUT_VERSION_REQUIRED", result.blockers);
            require(evidence.route_version != record.current_route_version,
                    "REPLAN_ROUTE_VERSION_NOT_ADVANCED", result.blockers);
            require(evidence.calculation_input_version != record.current_calculation_input_version,
                    "REPLAN_INPUT_VERSION_NOT_ADVANCED", result.blockers);
        }
        break;
    case MissionLifecycleState::Validating:
        require(evidence.baseline_route_generated, "BASELINE_ROUTE_NOT_GENERATED", result.blockers);
        require(evidence.weather_assessment_complete, "WEATHER_ASSESSMENT_NOT_COMPLETE", result.blockers);
        requireText(evidence.weather_snapshot_id, "WEATHER_SNAPSHOT_ID_REQUIRED", result.blockers);
        require(evidence.weather_aware_route_generated, "WEATHER_AWARE_ROUTE_NOT_GENERATED", result.blockers);
        require(evidence.notam_recheck_complete, "NOTAM_RECHECK_AFTER_WEATHER_REQUIRED", result.blockers);
        requireText(evidence.notam_recheck_snapshot_id, "NOTAM_RECHECK_SNAPSHOT_ID_REQUIRED", result.blockers);
        require(evidence.route_candidate_selected, "ROUTE_CANDIDATE_NOT_SELECTED", result.blockers);
        requireText(evidence.route_id, "SELECTED_ROUTE_ID_REQUIRED", result.blockers);
        requireText(evidence.route_version, "SELECTED_ROUTE_VERSION_REQUIRED", result.blockers);
        requireText(evidence.calculation_input_version, "CALCULATION_INPUT_VERSION_REQUIRED", result.blockers);
        break;
    case MissionLifecycleState::Ready:
        require(evidence.complete_trajectory_geometry, "COMPLETE_TRAJECTORY_GEOMETRY_REQUIRED", result.blockers);
        require(evidence.notam_validation_passed, "NOTAM_VALIDATION_NOT_PASSED", result.blockers);
        require(evidence.terrain_validation_passed, "TERRAIN_VALIDATION_NOT_PASSED", result.blockers);
        require(evidence.vehicle_envelope_validation_passed, "VEHICLE_ENVELOPE_VALIDATION_NOT_PASSED", result.blockers);
        require(evidence.weather_validation_passed, "WEATHER_VALIDATION_NOT_PASSED", result.blockers);
        require(evidence.energy_reserve_validation_passed, "ENERGY_RESERVE_VALIDATION_NOT_PASSED", result.blockers);
        require(evidence.task_quality_validation_passed, "TASK_QUALITY_VALIDATION_NOT_PASSED", result.blockers);
        require(evidence.regulatory_validation_passed, "REGULATORY_VALIDATION_NOT_PASSED", result.blockers);
        require(evidence.operator_review_accepted, "OPERATOR_REVIEW_NOT_ACCEPTED", result.blockers);
        require(evidence.validation_route_version == evidence.route_version,
                "VALIDATION_ROUTE_VERSION_MISMATCH", result.blockers);
        require(evidence.validation_input_version == evidence.calculation_input_version,
                "VALIDATION_INPUT_VERSION_MISMATCH", result.blockers);
        break;
    case MissionLifecycleState::Submitted:
        require(evidence.regulatory_submission_prepared, "REGULATORY_SUBMISSION_NOT_PREPARED", result.blockers);
        requireText(evidence.regulatory_submission_id, "REGULATORY_SUBMISSION_ID_REQUIRED", result.blockers);
        break;
    case MissionLifecycleState::Authorized:
        require(evidence.authorization_present, "EXTERNAL_AUTHORIZATION_MISSING", result.blockers);
        require(evidence.authorization_current, "EXTERNAL_AUTHORIZATION_STALE_OR_INVALID", result.blockers);
        require(evidence.authorization_scope_matches, "EXTERNAL_AUTHORIZATION_SCOPE_MISMATCH", result.blockers);
        requireText(evidence.authorization_id, "AUTHORIZATION_ID_REQUIRED", result.blockers);
        break;
    case MissionLifecycleState::Released:
        require(evidence.package_integrity_verified, "MISSION_PACKAGE_INTEGRITY_NOT_VERIFIED", result.blockers);
        require(evidence.package_matches_selected_route, "MISSION_PACKAGE_ROUTE_MISMATCH", result.blockers);
        require(evidence.package_route_version == evidence.route_version,
                "MISSION_PACKAGE_ROUTE_VERSION_MISMATCH", result.blockers);
        requireText(evidence.package_id, "MISSION_PACKAGE_ID_REQUIRED", result.blockers);
        require(evidence.package_vehicle_approved, "PACKAGE_VEHICLE_NOT_APPROVED", result.blockers);
        requireText(evidence.package_vehicle_id, "PACKAGE_VEHICLE_ID_REQUIRED", result.blockers);
        break;
    case MissionLifecycleState::Executing:
        require(evidence.execution_session_linked, "EXECUTION_SESSION_NOT_LINKED", result.blockers);
        requireText(evidence.execution_reference_id, "EXECUTION_REFERENCE_ID_REQUIRED", result.blockers);
        requireText(evidence.execution_vehicle_id, "EXECUTION_VEHICLE_ID_REQUIRED", result.blockers);
        require(evidence.execution_vehicle_id == evidence.package_vehicle_id,
                "EXECUTION_VEHICLE_PACKAGE_MISMATCH", result.blockers);
        break;
    case MissionLifecycleState::Completed:
        require(evidence.execution_closed, "EXECUTION_NOT_CLOSED", result.blockers);
        require(evidence.postflight_data_archived, "POSTFLIGHT_DATA_NOT_ARCHIVED", result.blockers);
        requireText(evidence.postflight_record_id, "POSTFLIGHT_RECORD_ID_REQUIRED", result.blockers);
        break;
    case MissionLifecycleState::Paused:
    case MissionLifecycleState::Replanning:
    case MissionLifecycleState::Aborted:
    case MissionLifecycleState::Draft:
        break;
    }

    if (!result.blockers.empty())
        return result;

    MissionLifecycleEvent event;
    event.sequence = record.next_event_sequence;
    event.mission_id = record.mission_id;
    event.mission_version = startsNewRevision ? evidence.mission_version : record.mission_version;
    event.parent_mission_version = startsNewRevision ? record.mission_version : record.parent_mission_version;
    event.actor_id = actor_id;
    event.timestamp = timestamp;
    event.reason = reason;
    event.from = record.state;
    event.to = target;
    event.route_id = evidence.route_id;
    event.route_version = evidence.route_version;
    event.evidence_reference = evidence.calculation_input_version;
    const auto addEvidence = [&event](const std::string &value) {
        if (!value.empty())
            event.evidence_references.push_back(value);
    };
    addEvidence(evidence.terrain_snapshot_id);
    addEvidence(evidence.mission_notam_snapshot_id);
    addEvidence(evidence.weather_snapshot_id);
    addEvidence(evidence.notam_recheck_snapshot_id);
    addEvidence(evidence.regulatory_submission_id);
    addEvidence(evidence.authorization_id);
    addEvidence(evidence.package_id);
    addEvidence(evidence.execution_reference_id);
    addEvidence(evidence.postflight_record_id);
    record.history.push_back(std::move(event));
    ++record.next_event_sequence;
    if (startsNewRevision) {
        record.parent_mission_version = record.mission_version;
        record.mission_version = evidence.mission_version;
    }
    if (!evidence.route_id.empty())
        record.current_route_id = evidence.route_id;
    if (!evidence.route_version.empty())
        record.current_route_version = evidence.route_version;
    if (!evidence.calculation_input_version.empty())
        record.current_calculation_input_version = evidence.calculation_input_version;
    record.state = target;

    result.accepted = true;
    result.state = record.state;
    return result;
}

} // namespace bluesky::planning
