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

bool allowed(MissionLifecycleState from, MissionLifecycleState to, MissionExecutionMode mode) noexcept {
    if (to == MissionLifecycleState::Aborted && !isTerminal(from))
        return true;
    switch (from) {
    case MissionLifecycleState::Draft: return to == MissionLifecycleState::Calculating;
    case MissionLifecycleState::Calculating: return to == MissionLifecycleState::Validating;
    case MissionLifecycleState::Validating: return to == MissionLifecycleState::Ready;
    case MissionLifecycleState::Ready:
        return mode == MissionExecutionMode::Virtual
            ? to == MissionLifecycleState::Released
            : to == MissionLifecycleState::Submitted;
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

const char *toString(MissionExecutionMode mode) noexcept {
    return mode == MissionExecutionMode::Virtual ? "VIRTUAL" : "PHYSICAL";
}

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

std::vector<std::string> MissionLifecycleController::mt01ReadinessBlockers(
    const MissionLifecycleRecord &record,
    const MissionLifecycleEvidence &evidence) {
    std::vector<std::string> blockers;
    if (std::find(record.mission_template_ids.begin(), record.mission_template_ids.end(), "MT-01")
        == record.mission_template_ids.end())
        return blockers;
    require(evidence.mt01_coverage_candidate_available,
            "MT01_COVERAGE_CANDIDATE_REQUIRED", blockers);
    require(evidence.mt01_flight_release_eligible,
            "MT01_FLIGHT_RELEASE_NOT_ELIGIBLE", blockers);
    require(evidence.mt01_task_execution_estimates_ready,
            "MT01_TASK_EXECUTION_ESTIMATES_NOT_READY", blockers);
    require(evidence.mt01_task_execution_estimates_applied_to_trajectory,
            "MT01_TASK_EXECUTION_ESTIMATES_NOT_APPLIED", blockers);
    require(evidence.mt01_camera_intrinsic_calibration_verified,
            "MT01_CAMERA_INTRINSIC_CALIBRATION_NOT_VERIFIED", blockers);
    return blockers;
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
    const bool initialModeBinding = record.state == MissionLifecycleState::Draft
        && target == MissionLifecycleState::Calculating;
    if (!initialModeBinding && evidence.execution_mode != record.execution_mode)
        result.blockers.emplace_back("MISSION_EXECUTION_MODE_CHANGED");
    if (initialModeBinding && evidence.execution_mode == MissionExecutionMode::Physical
        && evidence.vehicle_source_provenance != "PHYSICAL_LIVE")
        result.blockers.emplace_back("PHYSICAL_MODE_REQUIRES_LIVE_VEHICLE_PROVENANCE");
    if (initialModeBinding && evidence.execution_mode == MissionExecutionMode::Virtual
        && evidence.vehicle_source_provenance != "SIL_FIXTURE")
        result.blockers.emplace_back("VIRTUAL_MODE_REQUIRES_SIL_PROVENANCE");
    if (actor_id.empty())
        result.blockers.emplace_back("LIFECYCLE_ACTOR_REQUIRED");

    // Once the mission has an admitted vehicle assignment, every lifecycle
    // transition (including replanning) must carry the exact same identity.
    if (!record.vehicle_id.empty()) {
        require(evidence.vehicle_id == record.vehicle_id,
                "MISSION_VEHICLE_IDENTITY_CHANGED", result.blockers);
        require(evidence.vehicle_source_provenance == record.vehicle_source_provenance,
                "MISSION_VEHICLE_PROVENANCE_CHANGED", result.blockers);
        require(evidence.reference_model_id == record.reference_model_id,
                "MISSION_VEHICLE_MODEL_CHANGED", result.blockers);
        require(evidence.reference_type_id == record.reference_type_id,
                "MISSION_VEHICLE_TYPE_CHANGED", result.blockers);
        require(evidence.vehicle_configuration_id == record.vehicle_configuration_id,
                "MISSION_VEHICLE_CONFIGURATION_CHANGED", result.blockers);
        require(evidence.vehicle_configuration_revision == record.vehicle_configuration_revision,
                "MISSION_VEHICLE_CONFIGURATION_REVISION_CHANGED", result.blockers);
        require(evidence.fleet_assignment_snapshot_id == record.fleet_assignment_snapshot_id,
                "MISSION_FLEET_ASSIGNMENT_SNAPSHOT_CHANGED", result.blockers);
        require(evidence.performance_profile_id == record.performance_profile_id
                    && evidence.performance_profile_revision == record.performance_profile_revision,
                "MISSION_PERFORMANCE_PROFILE_CHANGED", result.blockers);
    }
    if (timestamp.empty())
        result.blockers.emplace_back("LIFECYCLE_TIMESTAMP_REQUIRED");
    if (!allowed(record.state, target, evidence.execution_mode))
        result.blockers.emplace_back("MISSION_LIFECYCLE_TRANSITION_NOT_ALLOWED");

    if (target == MissionLifecycleState::Aborted) {
        requireText(reason, "ABORT_REASON_REQUIRED", result.blockers);
        if (record.state == MissionLifecycleState::Executing) {
            require(evidence.execution_closed, "ABORTED_EXECUTION_NOT_CLOSED", result.blockers);
            require(evidence.postflight_data_archived, "ABORTED_POSTFLIGHT_DATA_NOT_ARCHIVED", result.blockers);
            requireText(evidence.postflight_record_id, "ABORTED_POSTFLIGHT_RECORD_ID_REQUIRED", result.blockers);
        }
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
            requireText(evidence.vehicle_configuration_id, "VEHICLE_CONFIGURATION_ID_REQUIRED", result.blockers);
            requireText(evidence.vehicle_id, "PHYSICAL_VEHICLE_ID_REQUIRED", result.blockers);
            require(evidence.vehicle_source_provenance == "PHYSICAL_LIVE"
                        || evidence.vehicle_source_provenance == "SIL_FIXTURE",
                    "VEHICLE_SOURCE_PROVENANCE_NOT_ADMITTED", result.blockers);
            requireText(evidence.reference_model_id, "VEHICLE_REFERENCE_MODEL_ID_REQUIRED", result.blockers);
            requireText(evidence.reference_type_id, "VEHICLE_REFERENCE_TYPE_ID_REQUIRED", result.blockers);
            requireText(evidence.vehicle_configuration_revision,
                        "VEHICLE_CONFIGURATION_REVISION_REQUIRED", result.blockers);
            requireText(evidence.fleet_assignment_snapshot_id,
                        "FLEET_ASSIGNMENT_SNAPSHOT_REQUIRED", result.blockers);
            requireText(evidence.performance_profile_id, "APPROVED_PERFORMANCE_PROFILE_ID_REQUIRED", result.blockers);
            requireText(evidence.performance_profile_revision,
                        "APPROVED_PERFORMANCE_PROFILE_REVISION_REQUIRED", result.blockers);
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
    case MissionLifecycleState::Ready: {
        const auto mt01Blockers = mt01ReadinessBlockers(record, evidence);
        result.blockers.insert(result.blockers.end(), mt01Blockers.begin(), mt01Blockers.end());
        const auto requireValidation = [&](const MissionLifecycleValidationEvidence &validation,
                                           const char *missingCode, const char *lineageCode) {
            const bool hasResult = validation.status == MissionLifecycleValidationStatus::Passed
                && !validation.result_id.empty() && !validation.validator_id.empty()
                && !validation.validator_version.empty()
                && !validation.source_reference.empty();
            require(hasResult, missingCode, result.blockers);
            if (hasResult) {
                require(validation.mission_id == record.mission_id
                        && validation.mission_version == record.mission_version
                        && validation.route_id == evidence.route_id
                        && validation.route_version == evidence.route_version
                        && validation.calculation_input_version == evidence.calculation_input_version
                        && evidence.route_id == record.current_route_id
                        && evidence.route_version == record.current_route_version
                        && evidence.calculation_input_version == record.current_calculation_input_version,
                        lineageCode, result.blockers);
            }
        };
        requireValidation(evidence.route_structure_validation,
                          "ROUTE_STRUCTURE_VALIDATION_EVIDENCE_REQUIRED",
                          "ROUTE_STRUCTURE_VALIDATION_LINEAGE_MISMATCH");
        requireValidation(evidence.trajectory_geometry_validation,
                          "TRAJECTORY_GEOMETRY_VALIDATION_EVIDENCE_REQUIRED",
                          "TRAJECTORY_GEOMETRY_VALIDATION_LINEAGE_MISMATCH");
        requireValidation(evidence.final_planning_integrity_validation,
                          "FINAL_PLANNING_INTEGRITY_EVIDENCE_REQUIRED",
                          "FINAL_PLANNING_INTEGRITY_LINEAGE_MISMATCH");
        require(evidence.final_planning_integrity_validation.result_id
                    == evidence.final_planning_integrity_validation.source_reference,
                "FINAL_PLANNING_INTEGRITY_DEPENDENCY_IDENTITY_MISMATCH", result.blockers);
        require(evidence.final_planning_integrity_validation.vehicle_configuration_id
                    == evidence.vehicle_configuration_id,
                "FINAL_PLANNING_INTEGRITY_VEHICLE_CONFIGURATION_MISMATCH", result.blockers);
        requireValidation(evidence.notam_validation,
                          "NOTAM_VALIDATION_EVIDENCE_REQUIRED", "NOTAM_VALIDATION_LINEAGE_MISMATCH");
        requireText(evidence.notam_validation.snapshot_id,
                    "NOTAM_VALIDATION_SNAPSHOT_ID_REQUIRED", result.blockers);
        require(evidence.notam_validation.snapshot_id == evidence.notam_recheck_snapshot_id,
                "NOTAM_VALIDATION_SNAPSHOT_MISMATCH", result.blockers);
        requireValidation(evidence.terrain_validation,
                          "TERRAIN_VALIDATION_EVIDENCE_REQUIRED", "TERRAIN_VALIDATION_LINEAGE_MISMATCH");
        requireValidation(evidence.vehicle_envelope_validation,
                          "VEHICLE_ENVELOPE_VALIDATION_EVIDENCE_REQUIRED",
                          "VEHICLE_ENVELOPE_VALIDATION_LINEAGE_MISMATCH");
        require(evidence.vehicle_envelope_validation.vehicle_configuration_id
                    == evidence.vehicle_configuration_id,
                "VEHICLE_ENVELOPE_CONFIGURATION_MISMATCH", result.blockers);
        requireValidation(evidence.weather_validation,
                          "WEATHER_VALIDATION_EVIDENCE_REQUIRED", "WEATHER_VALIDATION_LINEAGE_MISMATCH");
        requireText(evidence.weather_validation.snapshot_id,
                    "WEATHER_VALIDATION_SNAPSHOT_ID_REQUIRED", result.blockers);
        require(evidence.weather_validation.snapshot_id == evidence.weather_snapshot_id,
                "WEATHER_VALIDATION_SNAPSHOT_MISMATCH", result.blockers);
        requireValidation(evidence.energy_reserve_validation,
                          "ENERGY_RESERVE_VALIDATION_EVIDENCE_REQUIRED",
                          "ENERGY_RESERVE_VALIDATION_LINEAGE_MISMATCH");
        requireText(evidence.energy_reserve_validation.snapshot_id,
                    "ENERGY_RESERVE_VALIDATION_SNAPSHOT_ID_REQUIRED", result.blockers);
        require(evidence.energy_reserve_validation.snapshot_id == evidence.weather_snapshot_id,
                "ENERGY_RESERVE_VALIDATION_SNAPSHOT_MISMATCH", result.blockers);
        requireValidation(evidence.task_quality_validation,
                          "TASK_QUALITY_VALIDATION_EVIDENCE_REQUIRED",
                          "TASK_QUALITY_VALIDATION_LINEAGE_MISMATCH");
        requireValidation(evidence.regulatory_validation,
                          "REGULATORY_VALIDATION_EVIDENCE_REQUIRED",
                          "REGULATORY_VALIDATION_LINEAGE_MISMATCH");
        require(evidence.operator_review_accepted, "OPERATOR_REVIEW_NOT_ACCEPTED", result.blockers);
        break;
    }
    case MissionLifecycleState::Submitted:
        require(evidence.regulatory_submission_prepared, "REGULATORY_SUBMISSION_NOT_PREPARED", result.blockers);
        requireText(evidence.regulatory_submission_id, "REGULATORY_SUBMISSION_ID_REQUIRED", result.blockers);
        break;
    case MissionLifecycleState::Authorized:
        require(evidence.authorization_requirement_assessed,
                "AUTHORIZATION_APPLICABILITY_NOT_ASSESSED", result.blockers);
        if (evidence.authorization_required) {
            require(evidence.authorization_present, "EXTERNAL_AUTHORIZATION_MISSING", result.blockers);
            require(evidence.authorization_current, "EXTERNAL_AUTHORIZATION_STALE_OR_INVALID", result.blockers);
            require(evidence.authorization_scope_matches, "EXTERNAL_AUTHORIZATION_SCOPE_MISMATCH", result.blockers);
            requireText(evidence.authorization_id, "AUTHORIZATION_ID_REQUIRED", result.blockers);
        } else {
            requireText(evidence.authorization_not_required_reason,
                        "AUTHORIZATION_NOT_REQUIRED_REASON_MISSING", result.blockers);
        }
        break;
    case MissionLifecycleState::Released:
        if (evidence.execution_mode == MissionExecutionMode::Virtual) {
            require(record.state == MissionLifecycleState::Ready,
                    "VIRTUAL_RELEASE_MUST_FOLLOW_VIRTUAL_READY", result.blockers);
            require(evidence.authorization_requirement_assessed,
                    "VIRTUAL_AUTHORIZATION_APPLICABILITY_NOT_ASSESSED", result.blockers);
            require(!evidence.authorization_required,
                    "VIRTUAL_MODE_MUST_NOT_REQUEST_EXTERNAL_AUTHORIZATION", result.blockers);
            requireText(evidence.authorization_not_required_reason,
                        "VIRTUAL_NO_EXTERNAL_AUTHORIZATION_REASON_REQUIRED", result.blockers);
            require(evidence.authorization_not_required_reason
                        == "VIRTUAL_SANDBOX_NO_ESORVD_SUBMISSION_OR_AUTHORIZATION",
                    "VIRTUAL_EXTERNAL_AUTHORIZATION_REASON_INVALID", result.blockers);
            require(!evidence.regulatory_submission_prepared
                        && evidence.regulatory_submission_id.empty(),
                    "VIRTUAL_ESORVD_SUBMISSION_FORBIDDEN", result.blockers);
            require(!evidence.authorization_present && !evidence.authorization_current
                        && evidence.authorization_id.empty(),
                    "VIRTUAL_EXTERNAL_AUTHORIZATION_FORBIDDEN", result.blockers);
        } else {
            require(record.state == MissionLifecycleState::Authorized,
                    "PHYSICAL_RELEASE_REQUIRES_AUTHORIZED_STATE", result.blockers);
        }
        require(evidence.package_integrity_verified, "MISSION_PACKAGE_INTEGRITY_NOT_VERIFIED", result.blockers);
        require(evidence.package_matches_selected_route, "MISSION_PACKAGE_ROUTE_MISMATCH", result.blockers);
        requireText(evidence.package_route_id, "MISSION_PACKAGE_ROUTE_ID_REQUIRED", result.blockers);
        require(evidence.package_route_id == record.current_route_id,
                "MISSION_PACKAGE_ROUTE_ID_MISMATCH", result.blockers);
        require(evidence.package_route_version == evidence.route_version,
                "MISSION_PACKAGE_ROUTE_VERSION_MISMATCH", result.blockers);
        require(evidence.route_id == record.current_route_id,
                "RELEASE_ROUTE_ID_NOT_CURRENT", result.blockers);
        require(evidence.route_version == record.current_route_version,
                "RELEASE_ROUTE_VERSION_NOT_CURRENT", result.blockers);
        require(evidence.calculation_input_version == record.current_calculation_input_version,
                "RELEASE_INPUT_VERSION_NOT_CURRENT", result.blockers);
        requireText(evidence.package_id, "MISSION_PACKAGE_ID_REQUIRED", result.blockers);
        require(evidence.package_vehicle_approved, "PACKAGE_VEHICLE_NOT_APPROVED", result.blockers);
        requireText(evidence.package_vehicle_id, "PACKAGE_VEHICLE_ID_REQUIRED", result.blockers);
        require(evidence.package_vehicle_id == record.vehicle_id,
                "MISSION_PACKAGE_VEHICLE_ASSIGNMENT_MISMATCH", result.blockers);
        require(evidence.package_vehicle_configuration_id == record.vehicle_configuration_id,
                "MISSION_PACKAGE_CONFIGURATION_MISMATCH", result.blockers);
        require(evidence.package_vehicle_configuration_revision == record.vehicle_configuration_revision,
                "MISSION_PACKAGE_CONFIGURATION_REVISION_MISMATCH", result.blockers);
        break;
    case MissionLifecycleState::Executing:
        require(evidence.execution_session_linked, "EXECUTION_SESSION_NOT_LINKED", result.blockers);
        requireText(evidence.execution_reference_id, "EXECUTION_REFERENCE_ID_REQUIRED", result.blockers);
        requireText(evidence.execution_vehicle_id, "EXECUTION_VEHICLE_ID_REQUIRED", result.blockers);
        require(evidence.execution_vehicle_id == evidence.package_vehicle_id,
                "EXECUTION_VEHICLE_PACKAGE_MISMATCH", result.blockers);
        require(evidence.execution_vehicle_id == record.vehicle_id,
                "EXECUTION_VEHICLE_ASSIGNMENT_MISMATCH", result.blockers);
        require(evidence.execution_vehicle_configuration_id == record.vehicle_configuration_id,
                "EXECUTION_VEHICLE_CONFIGURATION_MISMATCH", result.blockers);
        require(evidence.execution_vehicle_configuration_revision == record.vehicle_configuration_revision,
                "EXECUTION_VEHICLE_CONFIGURATION_REVISION_MISMATCH", result.blockers);
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
    event.execution_mode = evidence.execution_mode;
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
    event.vehicle_id = evidence.vehicle_id;
    event.vehicle_source_provenance = evidence.vehicle_source_provenance;
    event.reference_model_id = evidence.reference_model_id;
    event.reference_type_id = evidence.reference_type_id;
    event.vehicle_configuration_id = evidence.vehicle_configuration_id;
    event.vehicle_configuration_revision = evidence.vehicle_configuration_revision;
    event.fleet_assignment_snapshot_id = evidence.fleet_assignment_snapshot_id;
    event.performance_profile_id = evidence.performance_profile_id;
    event.performance_profile_revision = evidence.performance_profile_revision;
    const auto addEvidence = [&event](const std::string &value) {
        if (!value.empty())
            event.evidence_references.push_back(value);
    };
    addEvidence(evidence.vehicle_id);
    addEvidence(evidence.vehicle_source_provenance);
    addEvidence(evidence.reference_model_id);
    addEvidence(evidence.reference_type_id);
    addEvidence(evidence.vehicle_configuration_id);
    addEvidence(evidence.vehicle_configuration_revision);
    addEvidence(evidence.fleet_assignment_snapshot_id);
    addEvidence(evidence.performance_profile_id);
    addEvidence(evidence.performance_profile_revision);
    addEvidence(evidence.terrain_snapshot_id);
    addEvidence(evidence.mission_notam_snapshot_id);
    addEvidence(evidence.weather_snapshot_id);
    addEvidence(evidence.notam_recheck_snapshot_id);
    addEvidence(evidence.regulatory_submission_id);
    addEvidence(evidence.authorization_id);
    addEvidence(evidence.authorization_not_required_reason);
    addEvidence(evidence.package_id);
    addEvidence(evidence.execution_reference_id);
    addEvidence(evidence.postflight_record_id);
    record.history.push_back(std::move(event));
    ++record.next_event_sequence;
    if (startsNewRevision) {
        record.parent_mission_version = record.mission_version;
        record.mission_version = evidence.mission_version;
    }
    if (record.vehicle_id.empty() && record.state == MissionLifecycleState::Draft
        && target == MissionLifecycleState::Calculating) {
        record.execution_mode = evidence.execution_mode;
        record.vehicle_id = evidence.vehicle_id;
        record.vehicle_source_provenance = evidence.vehicle_source_provenance;
        record.reference_model_id = evidence.reference_model_id;
        record.reference_type_id = evidence.reference_type_id;
        record.vehicle_configuration_id = evidence.vehicle_configuration_id;
        record.vehicle_configuration_revision = evidence.vehicle_configuration_revision;
        record.fleet_assignment_snapshot_id = evidence.fleet_assignment_snapshot_id;
        record.performance_profile_id = evidence.performance_profile_id;
        record.performance_profile_revision = evidence.performance_profile_revision;
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
