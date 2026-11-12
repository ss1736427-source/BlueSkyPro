#include "mission_lifecycle.hpp"

#include <algorithm>
#include <cassert>
#include <string>

using namespace bluesky::planning;

static MissionLifecycleEvidence evidence() {
    MissionLifecycleEvidence e;
    e.mission_id = "MISSION-001";
    e.mission_version = "7";
    e.route_id = "ROUTE-01";
    e.route_version = "3";
    e.calculation_input_version = "INPUT-9";
    e.validation_route_version = "3";
    e.validation_input_version = "INPUT-9";
    e.terrain_snapshot_id = "TERRAIN-4";
    e.mission_notam_snapshot_id = "NOTAM-5";
    e.weather_snapshot_id = "GFS-20261112-12Z";
    e.notam_recheck_snapshot_id = "NOTAM-6";
    e.regulatory_submission_id = "SUBMISSION-1";
    e.authorization_id = "AUTH-1";
    e.package_id = "PACKAGE-1";
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
    e.complete_trajectory_geometry = true;
    e.notam_validation_passed = true;
    e.terrain_validation_passed = true;
    e.vehicle_envelope_validation_passed = true;
    e.weather_validation_passed = true;
    e.energy_reserve_validation_passed = true;
    e.task_quality_validation_passed = true;
    e.regulatory_validation_passed = true;
    e.operator_review_accepted = true;
    e.regulatory_submission_prepared = true;
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
    e.validation_route_version = "OLD-ROUTE";
    auto staleValidation = move(record, MissionLifecycleState::Ready, e);
    assert(!staleValidation.accepted);
    assert(std::find(staleValidation.blockers.begin(), staleValidation.blockers.end(),
                     "VALIDATION_ROUTE_VERSION_MISMATCH") != staleValidation.blockers.end());
    e.validation_route_version = e.route_version;
    assert(move(record, MissionLifecycleState::Ready, e).accepted);
    assert(move(record, MissionLifecycleState::Submitted, e).accepted);

    e.authorization_scope_matches = false;
    auto wrongScope = move(record, MissionLifecycleState::Authorized, e);
    assert(!wrongScope.accepted);
    e.authorization_scope_matches = true;
    assert(move(record, MissionLifecycleState::Authorized, e).accepted);

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
    assert(MissionLifecycleController::transition(
        record, MissionLifecycleState::Calculating, e, "OPERATOR-1",
        "2026-11-12T12:03:00Z", "replan").accepted);

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

    assert(record.next_event_sequence == record.history.size() + 1);
    assert(record.history.front().sequence == 1);
    assert(std::string(toString(record.state)) == "CALCULATING");
    return 0;
}
