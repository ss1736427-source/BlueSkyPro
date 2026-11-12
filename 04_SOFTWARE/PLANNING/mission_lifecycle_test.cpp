#include "mission_lifecycle.hpp"
#include "mission_lifecycle_route_adapter.hpp"
#include "mission_lifecycle_notam_adapter.hpp"

#include <algorithm>
#include <cassert>
#include <string>

using namespace bluesky::planning;

static MissionLifecycleValidationEvidence routeStructureValidation() {
    Route route;
    route.lineage = {"ROUTE-01", "3", "MISSION-001", "7",
                     "test-generator", "1", "INPUT-9"};
    route.constraints = {80.0, 120.0, 30.0, true};
    route.environment.airspace_snapshot_id = "NOTAM-6";
    route.waypoints = {
        {"WP-1", {60.0, 25.0}, 100.0, true, AltitudeReference::AGL},
        {"WP-2", {60.1, 25.1}, 100.0, false, AltitudeReference::AGL}
    };
    route.segments = {{"SEG-1", "WP-1", "WP-2", 1000.0, 20.0}};
    const auto result = RouteConstraintValidator::validate(route);
    assert(result.status == RouteValidationStatus::Allowed);
    return makeRouteStructureValidationEvidence(route, result);
}

static MissionLifecycleValidationEvidence notamValidation() {
    Route route;
    route.lineage = {"ROUTE-01", "3", "MISSION-001", "7",
                     "test-generator", "1", "INPUT-9"};
    route.constraints = {80.0, 120.0, 30.0, true};
    route.environment.airspace_snapshot_id = "NOTAM-6";
    route.waypoints = {
        {"WP-1", {60.0, 25.0}, 100.0, true, AltitudeReference::AGL},
        {"WP-2", {60.1, 25.1}, 100.0, false, AltitudeReference::AGL}
    };
    route.segments = {{"SEG-1", "WP-1", "WP-2", 1000.0, 20.0}};
    NotamSnapshot snapshot;
    snapshot.snapshot_id = "NOTAM-6";
    snapshot.source_id = "TEST-NOTAM-SOURCE";
    snapshot.source_version = "1";
    snapshot.evaluation_epoch_s = 1000;
    const auto result = NotamProhibitedZoneValidator::validate(route, snapshot);
    assert(result.status == RouteValidationStatus::Allowed);
    auto evidence = makeNotamValidationEvidence(route, snapshot, result);
    assert(evidence.status == MissionLifecycleValidationStatus::Passed);

    auto mismatchedSnapshot = snapshot;
    mismatchedSnapshot.snapshot_id = "NOTAM-OTHER";
    const auto rejected = NotamProhibitedZoneValidator::validate(route, mismatchedSnapshot);
    const auto rejectedEvidence = makeNotamValidationEvidence(route, mismatchedSnapshot, rejected);
    assert(rejectedEvidence.status == MissionLifecycleValidationStatus::Failed);
    return evidence;
}

static MissionLifecycleValidationEvidence validation(const std::string &id) {
    MissionLifecycleValidationEvidence result;
    result.result_id = id;
    result.mission_id = "MISSION-001";
    result.mission_version = "7";
    result.route_id = "ROUTE-01";
    result.route_version = "3";
    result.calculation_input_version = "INPUT-9";
    result.validator_id = "validator-" + id;
    result.validator_version = "1";
    result.source_reference = "test-fixture:" + id;
    result.status = MissionLifecycleValidationStatus::Passed;
    return result;
}

static MissionLifecycleEvidence evidence() {
    MissionLifecycleEvidence e;
    e.mission_id = "MISSION-001";
    e.mission_version = "7";
    e.route_id = "ROUTE-01";
    e.route_version = "3";
    e.calculation_input_version = "INPUT-9";
    e.terrain_snapshot_id = "TERRAIN-4";
    e.mission_notam_snapshot_id = "NOTAM-5";
    e.weather_snapshot_id = "GFS-20261112-12Z";
    e.notam_recheck_snapshot_id = "NOTAM-6";
    e.regulatory_submission_id = "SUBMISSION-1";
    e.authorization_id = "AUTH-1";
    e.package_id = "PACKAGE-1";
    e.package_route_id = "ROUTE-01";
    e.package_route_version = "3";
    e.package_vehicle_id = "UAV-1";
    e.execution_reference_id = "EXEC-1";
    e.postflight_record_id = "POSTFLIGHT-1";
    e.execution_vehicle_id = "UAV-1";
    e.task_definition_complete = true;
    e.mission_geometry_complete = true;
    e.vehicle_configuration_validated = true;
    e.equipment_configuration_validated = true;
    e.mission_notam_snapshot_current = true;
    e.terrain_snapshot_current = true;
    e.baseline_route_generated = true;
    e.weather_assessment_complete = true;
    e.weather_aware_route_generated = true;
    e.notam_recheck_complete = true;
    e.route_candidate_selected = true;
    e.route_structure_validation = routeStructureValidation();
    e.trajectory_geometry_validation = validation("GEOMETRY-RESULT");
    e.notam_validation = notamValidation();
    e.terrain_validation = validation("TERRAIN-RESULT");
    e.vehicle_envelope_validation = validation("VEHICLE-RESULT");
    e.weather_validation = validation("WEATHER-RESULT");
    e.energy_reserve_validation = validation("ENERGY-RESULT");
    e.task_quality_validation = validation("TASK-QUALITY-RESULT");
    e.regulatory_validation = validation("REGULATORY-RESULT");
    e.operator_review_accepted = true;
    e.regulatory_submission_prepared = true;
    e.authorization_requirement_assessed = true;
    e.authorization_required = true;
    e.authorization_present = true;
    e.authorization_current = true;
    e.authorization_scope_matches = true;
    e.package_integrity_verified = true;
    e.package_matches_selected_route = true;
    e.package_vehicle_approved = true;
    e.execution_session_linked = true;
    e.execution_closed = true;
    e.postflight_data_archived = true;
    return e;
}

static MissionLifecycleTransitionResult move(
    MissionLifecycleRecord &record, MissionLifecycleState target,
    const MissionLifecycleEvidence &e) {
    return MissionLifecycleController::transition(
        record, target, e, "OPERATOR-1", "2026-11-12T12:00:00Z", "test transition");
}

int main() {
    MissionLifecycleRecord record{"MISSION-001", "7"};
    auto e = evidence();
    e.task_definition_complete = false;

    auto blocked = move(record, MissionLifecycleState::Calculating, e);
    assert(!blocked.accepted);
    assert(record.state == MissionLifecycleState::Draft);
    assert(record.history.empty());
    assert(std::find(blocked.blockers.begin(), blocked.blockers.end(),
                     "TASK_DEFINITION_INCOMPLETE") != blocked.blockers.end());

    e.task_definition_complete = true;
    auto started = move(record, MissionLifecycleState::Calculating, e);
    assert(started.accepted && record.state == MissionLifecycleState::Calculating);

    auto skipped = move(record, MissionLifecycleState::Ready, e);
    assert(!skipped.accepted);
    assert(record.state == MissionLifecycleState::Calculating);

    assert(move(record, MissionLifecycleState::Validating, e).accepted);
    e.notam_validation.route_version = "OLD-ROUTE";
    auto staleValidation = move(record, MissionLifecycleState::Ready, e);
    assert(!staleValidation.accepted);
    assert(std::find(staleValidation.blockers.begin(), staleValidation.blockers.end(),
                     "NOTAM_VALIDATION_LINEAGE_MISMATCH") != staleValidation.blockers.end());
    e.notam_validation = notamValidation();
    auto missingRouteStructure = e;
    missingRouteStructure.route_structure_validation.result_id.clear();
    auto missingRouteStructureResult = move(record, MissionLifecycleState::Ready, missingRouteStructure);
    assert(!missingRouteStructureResult.accepted);
    assert(std::find(missingRouteStructureResult.blockers.begin(), missingRouteStructureResult.blockers.end(),
                     "ROUTE_STRUCTURE_VALIDATION_EVIDENCE_REQUIRED") != missingRouteStructureResult.blockers.end());
    auto missingValidationId = e;
    missingValidationId.terrain_validation.result_id.clear();
    auto missingValidation = move(record, MissionLifecycleState::Ready, missingValidationId);
    assert(!missingValidation.accepted);
    assert(std::find(missingValidation.blockers.begin(), missingValidation.blockers.end(),
                     "TERRAIN_VALIDATION_EVIDENCE_REQUIRED") != missingValidation.blockers.end());
    auto changedRouteIdentity = e;
    changedRouteIdentity.route_id = "ROUTE-OTHER";
    auto wrongRouteIdentity = move(record, MissionLifecycleState::Ready, changedRouteIdentity);
    assert(!wrongRouteIdentity.accepted);
    assert(std::find(wrongRouteIdentity.blockers.begin(), wrongRouteIdentity.blockers.end(),
                     "VALIDATION_ROUTE_ID_NOT_CURRENT") != wrongRouteIdentity.blockers.end());
    auto changedInputIdentity = e;
    changedInputIdentity.calculation_input_version = "INPUT-OTHER";
    auto wrongInputIdentity = move(record, MissionLifecycleState::Ready, changedInputIdentity);
    assert(!wrongInputIdentity.accepted);
    assert(std::find(wrongInputIdentity.blockers.begin(), wrongInputIdentity.blockers.end(),
                     "NOTAM_VALIDATION_LINEAGE_MISMATCH") != wrongInputIdentity.blockers.end());
    assert(move(record, MissionLifecycleState::Ready, e).accepted);
    assert(move(record, MissionLifecycleState::Submitted, e).accepted);

    e.authorization_scope_matches = false;
    auto wrongScope = move(record, MissionLifecycleState::Authorized, e);
    assert(!wrongScope.accepted);
    e.authorization_scope_matches = true;
    assert(move(record, MissionLifecycleState::Authorized, e).accepted);

    e.package_route_id = "ROUTE-OTHER";
    auto wrongPackageRouteId = move(record, MissionLifecycleState::Released, e);
    assert(!wrongPackageRouteId.accepted);
    assert(std::find(wrongPackageRouteId.blockers.begin(), wrongPackageRouteId.blockers.end(),
                     "MISSION_PACKAGE_ROUTE_ID_MISMATCH") != wrongPackageRouteId.blockers.end());
    e.package_route_id = "ROUTE-01";
    e.route_id = "ROUTE-OTHER";
    auto changedReleaseRoute = move(record, MissionLifecycleState::Released, e);
    assert(!changedReleaseRoute.accepted);
    assert(std::find(changedReleaseRoute.blockers.begin(), changedReleaseRoute.blockers.end(),
                     "RELEASE_ROUTE_ID_NOT_CURRENT") != changedReleaseRoute.blockers.end());
    e.route_id = "ROUTE-01";
    e.calculation_input_version = "INPUT-OTHER";
    auto changedReleaseInput = move(record, MissionLifecycleState::Released, e);
    assert(!changedReleaseInput.accepted);
    assert(std::find(changedReleaseInput.blockers.begin(), changedReleaseInput.blockers.end(),
                     "RELEASE_INPUT_VERSION_NOT_CURRENT") != changedReleaseInput.blockers.end());
    e.calculation_input_version = "INPUT-9";
    e.package_route_version = "OLD-ROUTE";
    auto wrongPackage = move(record, MissionLifecycleState::Released, e);
    assert(!wrongPackage.accepted);
    e.package_route_version = e.route_version;
    assert(move(record, MissionLifecycleState::Released, e).accepted);

    e.execution_vehicle_id = "UAV-OTHER";
    assert(!move(record, MissionLifecycleState::Executing, e).accepted);
    e.execution_vehicle_id = e.package_vehicle_id;
    assert(move(record, MissionLifecycleState::Executing, e).accepted);

    auto pauseWithoutReason = MissionLifecycleController::transition(
        record, MissionLifecycleState::Paused, e, "OPERATOR-1",
        "2026-11-12T12:01:00Z", "");
    assert(!pauseWithoutReason.accepted);
    assert(MissionLifecycleController::transition(
        record, MissionLifecycleState::Paused, e, "OPERATOR-1",
        "2026-11-12T12:01:00Z", "weather deviation").accepted);
    assert(MissionLifecycleController::transition(
        record, MissionLifecycleState::Replanning, e, "OPERATOR-1",
        "2026-11-12T12:02:00Z", "weather deviation").accepted);
    auto staleReplan = e;
    staleReplan.mission_version = "8";
    auto staleReplanResult = MissionLifecycleController::transition(
        record, MissionLifecycleState::Calculating, staleReplan, "OPERATOR-1",
        "2026-11-12T12:03:00Z", "replan");
    assert(!staleReplanResult.accepted);
    assert(std::find(staleReplanResult.blockers.begin(), staleReplanResult.blockers.end(),
                     "REPLAN_ROUTE_VERSION_NOT_ADVANCED") != staleReplanResult.blockers.end());

    auto replan = e;
    replan.mission_version = "8";
    replan.route_version = "4";
    replan.calculation_input_version = "INPUT-10";
    assert(MissionLifecycleController::transition(
        record, MissionLifecycleState::Calculating, replan, "OPERATOR-1",
        "2026-11-12T12:03:00Z", "replan").accepted);
    assert(record.mission_version == "8");
    assert(record.parent_mission_version == "7");
    assert(record.history.back().parent_mission_version == "7");
    assert(record.history.back().mission_version == "8");

    const auto eventCount = record.history.size();
    auto wrongMission = e;
    wrongMission.mission_version = "6";
    auto mismatch = move(record, MissionLifecycleState::Validating, wrongMission);
    assert(!mismatch.accepted);
    assert(record.history.size() == eventCount);

    MissionLifecycleRecord completed{"MISSION-001", "7"};
    assert(move(completed, MissionLifecycleState::Calculating, e).accepted);
    assert(move(completed, MissionLifecycleState::Validating, e).accepted);
    assert(move(completed, MissionLifecycleState::Ready, e).accepted);
    assert(move(completed, MissionLifecycleState::Submitted, e).accepted);
    assert(move(completed, MissionLifecycleState::Authorized, e).accepted);
    assert(move(completed, MissionLifecycleState::Released, e).accepted);
    assert(move(completed, MissionLifecycleState::Executing, e).accepted);
    e.execution_closed = false;
    auto prematureCompletion = move(completed, MissionLifecycleState::Completed, e);
    assert(!prematureCompletion.accepted);
    assert(std::find(prematureCompletion.blockers.begin(), prematureCompletion.blockers.end(),
                     "EXECUTION_NOT_CLOSED") != prematureCompletion.blockers.end());
    e.execution_closed = true;
    assert(move(completed, MissionLifecycleState::Completed, e).accepted);
    assert(completed.state == MissionLifecycleState::Completed);
    assert(!move(completed, MissionLifecycleState::Paused, e).accepted);
    assert(std::find(completed.history.back().evidence_references.begin(),
                     completed.history.back().evidence_references.end(), "POSTFLIGHT-1")
           != completed.history.back().evidence_references.end());

    MissionLifecycleRecord noAuthorizationRequired{"MISSION-001", "7"};
    assert(move(noAuthorizationRequired, MissionLifecycleState::Calculating, e).accepted);
    assert(move(noAuthorizationRequired, MissionLifecycleState::Validating, e).accepted);
    assert(move(noAuthorizationRequired, MissionLifecycleState::Ready, e).accepted);
    assert(move(noAuthorizationRequired, MissionLifecycleState::Submitted, e).accepted);
    auto noAuthEvidence = e;
    noAuthEvidence.authorization_requirement_assessed = false;
    noAuthEvidence.authorization_required = false;
    noAuthEvidence.authorization_present = false;
    noAuthEvidence.authorization_id.clear();
    noAuthEvidence.authorization_not_required_reason = "No separate authorization applies";
    auto unassessedAuthorization = move(noAuthorizationRequired, MissionLifecycleState::Authorized, noAuthEvidence);
    assert(!unassessedAuthorization.accepted);
    noAuthEvidence.authorization_requirement_assessed = true;
    assert(move(noAuthorizationRequired, MissionLifecycleState::Authorized, noAuthEvidence).accepted);

    MissionLifecycleRecord aborted{"MISSION-001", "7"};
    assert(move(aborted, MissionLifecycleState::Calculating, e).accepted);
    assert(move(aborted, MissionLifecycleState::Validating, e).accepted);
    assert(move(aborted, MissionLifecycleState::Ready, e).accepted);
    assert(move(aborted, MissionLifecycleState::Submitted, e).accepted);
    assert(move(aborted, MissionLifecycleState::Authorized, e).accepted);
    assert(move(aborted, MissionLifecycleState::Released, e).accepted);
    assert(move(aborted, MissionLifecycleState::Executing, e).accepted);
    auto abortEvidence = e;
    abortEvidence.execution_closed = false;
    auto incompleteAbort = MissionLifecycleController::transition(
        aborted, MissionLifecycleState::Aborted, abortEvidence, "OPERATOR-1",
        "2026-11-12T12:04:00Z", "operator abort");
    assert(!incompleteAbort.accepted);
    assert(std::find(incompleteAbort.blockers.begin(), incompleteAbort.blockers.end(),
                     "ABORTED_EXECUTION_NOT_CLOSED") != incompleteAbort.blockers.end());
    assert(MissionLifecycleController::transition(
        aborted, MissionLifecycleState::Aborted, e, "OPERATOR-1",
        "2026-11-12T12:05:00Z", "operator abort").accepted);

    assert(record.next_event_sequence == record.history.size() + 1);
    assert(record.history.front().sequence == 1);
    assert(std::string(toString(record.state)) == "CALCULATING");
    return 0;
}
