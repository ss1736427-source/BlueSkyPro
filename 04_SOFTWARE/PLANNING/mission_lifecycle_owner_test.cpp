#include "mission_lifecycle_owner.hpp"
#include "mission_lifecycle_validation_coordinator.hpp"

#include <cassert>
#include <string>
#include <vector>

using namespace bluesky::planning;

namespace {
Mission makeMission()
{
    Mission mission;
    mission.identity.mission_id = "OWNER-TEST-001";
    mission.identity.mission_version = "1";
    mission.selected_template_ids = {"MT-01"};
    return mission;
}

MissionLifecycleEvidence initialEvidence()
{
    MissionLifecycleEvidence evidence;
    evidence.execution_mode = MissionExecutionMode::Virtual;
    evidence.mission_id = "OWNER-TEST-001";
    evidence.mission_version = "1";
    evidence.vehicle_id = "SIL-M3E-001";
    evidence.vehicle_source_provenance = "SIL_FIXTURE";
    evidence.reference_model_id = "DJI-MAVIC-3E-CATALOGUE-REFERENCE-ONLY";
    evidence.reference_type_id = "MULTIROTOR";
    evidence.vehicle_configuration_id = "SIL-CFG-001";
    evidence.vehicle_configuration_revision = "1";
    evidence.fleet_assignment_snapshot_id = "FLEET-SNAPSHOT-001";
    evidence.performance_profile_id = "SIL-PERFORMANCE-001";
    evidence.performance_profile_revision = "1";
    evidence.mission_notam_snapshot_id = "NOTAM-SNAPSHOT-001";
    evidence.terrain_snapshot_id = "TERRAIN-SNAPSHOT-001";
    evidence.task_definition_complete = true;
    evidence.mission_geometry_complete = true;
    evidence.vehicle_configuration_validated = true;
    evidence.equipment_configuration_validated = true;
    evidence.mission_notam_snapshot_current = true;
    evidence.terrain_snapshot_current = true;
    return evidence;
}

void testRegistrationCreatesOneCanonicalDraft()
{
    MissionLifecycleOwner owner;
    std::vector<std::string> blockers;
    assert(owner.registerMission(makeMission(), &blockers));
    assert(blockers.empty());
    assert(owner.contains("OWNER-TEST-001"));

    const auto record = owner.snapshot("OWNER-TEST-001");
    assert(record.has_value());
    assert(record->mission_id == "OWNER-TEST-001");
    assert(record->mission_version == "1");
    assert(record->mission_template_ids.size() == 1);
    assert(record->mission_template_ids.front() == "MT-01");
    assert(record->state == MissionLifecycleState::Draft);
    assert(record->history.empty());
}

void testDuplicateAndPrePopulatedLifecycleAreRejected()
{
    MissionLifecycleOwner owner;
    assert(owner.registerMission(makeMission()));

    std::vector<std::string> blockers;
    assert(!owner.registerMission(makeMission(), &blockers));
    assert(blockers.size() == 1);
    assert(blockers.front() == "MISSION_ALREADY_REGISTERED");

    Mission invalid = makeMission();
    invalid.lifecycle.history.push_back(MissionLifecycleEvent{});
    blockers.clear();
    assert(!owner.registerMission(invalid, &blockers));
    assert(!blockers.empty());
    assert(blockers.front() == "MISSION_LIFECYCLE_MUST_START_AS_UNBOUND_DRAFT");
}

void testRejectedTransitionDoesNotMutateCanonicalRecord()
{
    MissionLifecycleOwner owner;
    assert(owner.registerMission(makeMission()));

    auto invalid = initialEvidence();
    invalid.vehicle_source_provenance = "PHYSICAL_LIVE";
    const auto rejected = owner.transition(
        "OWNER-TEST-001", MissionLifecycleState::Calculating,
        invalid, "test-actor", "2026-11-16T00:00:00Z", "invalid provenance");
    assert(!rejected.accepted);

    const auto unchanged = owner.snapshot("OWNER-TEST-001");
    assert(unchanged.has_value());
    assert(unchanged->state == MissionLifecycleState::Draft);
    assert(unchanged->history.empty());
    assert(unchanged->vehicle_id.empty());
}

void testAcceptedTransitionPinsModeAndVehicle()
{
    MissionLifecycleOwner owner;
    assert(owner.registerMission(makeMission()));

    const auto accepted = owner.transition(
        "OWNER-TEST-001", MissionLifecycleState::Calculating,
        initialEvidence(), "test-actor", "2026-11-16T00:00:00Z", "admit virtual mission");
    assert(accepted.accepted);
    assert(accepted.state == MissionLifecycleState::Calculating);

    const auto record = owner.snapshot("OWNER-TEST-001");
    assert(record.has_value());
    assert(record->execution_mode == MissionExecutionMode::Virtual);
    assert(record->vehicle_source_provenance == "SIL_FIXTURE");
    assert(record->vehicle_id == "SIL-M3E-001");
    assert(record->history.size() == 1);
    assert(record->history.front().execution_mode == MissionExecutionMode::Virtual);

    auto drifted = initialEvidence();
    drifted.vehicle_source_provenance = "PHYSICAL_LIVE";
    const auto rejected = owner.transition(
        "OWNER-TEST-001", MissionLifecycleState::Validating,
        drifted, "test-actor", "2026-11-16T00:01:00Z", "attempt provenance drift");
    assert(!rejected.accepted);

    const auto afterDrift = owner.snapshot("OWNER-TEST-001");
    assert(afterDrift.has_value());
    assert(afterDrift->state == MissionLifecycleState::Calculating);
    assert(afterDrift->history.size() == 1);
    assert(afterDrift->vehicle_source_provenance == "SIL_FIXTURE");
}

void testDraftRevisionAdvancesOnlyBeforeAdmission()
{
    MissionLifecycleOwner owner;
    Mission first = makeMission();
    assert(owner.synchronizeDraftMission(first));

    Mission rollback = makeMission();
    rollback.identity.mission_version = "0";
    rollback.lifecycle.mission_version = "0";
    std::vector<std::string> rollbackBlockers;
    assert(!owner.synchronizeDraftMission(rollback, &rollbackBlockers));
    assert(rollbackBlockers.size() == 1);
    assert(rollbackBlockers.front() == "MISSION_DRAFT_VERSION_NOT_ADVANCED");

    Mission revised = makeMission();
    revised.identity.mission_version = "2";
    revised.lifecycle.mission_version = "2";
    revised.selected_template_ids = {"MT-01", "MT-03"};
    assert(owner.synchronizeDraftMission(revised));

    const auto draft = owner.snapshot("OWNER-TEST-001");
    assert(draft.has_value());
    assert(draft->state == MissionLifecycleState::Draft);
    assert(draft->mission_version == "2");
    assert(draft->parent_mission_version == "1");
    assert(draft->mission_template_ids.size() == 2);
    assert(draft->history.empty());

    const auto admitted = owner.transition(
        "OWNER-TEST-001", MissionLifecycleState::Calculating,
        initialEvidence(), "test-actor", "2026-11-16T00:00:00Z", "admit mission");
    assert(!admitted.accepted); // evidence still names revision 1
    assert(owner.snapshot("OWNER-TEST-001")->mission_version == "2");

    auto revisionTwoEvidence = initialEvidence();
    revisionTwoEvidence.mission_version = "2";
    const auto accepted = owner.transition(
        "OWNER-TEST-001", MissionLifecycleState::Calculating,
        revisionTwoEvidence, "test-actor", "2026-11-16T00:00:01Z", "admit revision 2");
    assert(accepted.accepted);

    Mission afterAdmission = revised;
    afterAdmission.identity.mission_version = "3";
    afterAdmission.lifecycle.mission_version = "3";
    std::vector<std::string> blockers;
    assert(!owner.synchronizeDraftMission(afterAdmission, &blockers));
    assert(blockers.size() == 1);
    assert(blockers.front() == "MISSION_DRAFT_REVISION_NOT_ALLOWED_AFTER_ADMISSION");
    assert(owner.snapshot("OWNER-TEST-001")->mission_version == "2");
}


void testOwnerDoesNotCommitWhenAuthoritativeSourcesAreMissing()
{
    MissionLifecycleOwner owner;
    assert(owner.registerMission(makeMission()));
    MissionLifecycleAuthoritativeSources sources;
    RegulatoryComplianceAssessment regulatory;
    FlightProfile profile;
    VehicleCompilationInput vehicle;
    const auto result = owner.finalizeReadyFromAuthoritativeSources(
        "OWNER-TEST-001", initialEvidence(), sources, regulatory, 1000,
        profile, vehicle, "test-actor", "2026-11-16T00:00:00Z",
        "missing authoritative sources");
    assert(!result.accepted);
    assert(!result.blockers.empty());
    const auto canonical = owner.snapshot("OWNER-TEST-001");
    assert(canonical.has_value());
    assert(canonical->state == MissionLifecycleState::Draft);
    assert(canonical->history.empty());
}

void testUnknownMissionCannotBeTransitioned()
{
    MissionLifecycleOwner owner;
    const auto result = owner.transition(
        "UNKNOWN", MissionLifecycleState::Calculating, {},
        "test-actor", "2026-11-16T00:00:00Z", "must fail");
    assert(!result.accepted);
    assert(result.blockers.size() == 1);
    assert(result.blockers.front() == "MISSION_NOT_REGISTERED");
}
}

int main()
{
    testRegistrationCreatesOneCanonicalDraft();
    testDuplicateAndPrePopulatedLifecycleAreRejected();
    testRejectedTransitionDoesNotMutateCanonicalRecord();
    testAcceptedTransitionPinsModeAndVehicle();
    testDraftRevisionAdvancesOnlyBeforeAdmission();
    testOwnerDoesNotCommitWhenAuthoritativeSourcesAreMissing();
    testUnknownMissionCannotBeTransitioned();
    return 0;
}
