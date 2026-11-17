#include "mission_lifecycle_validation_coordinator.hpp"
#include <algorithm>
#include <cassert>
using namespace bluesky::planning;
static Route routeFixture() {
    Route r;r.lineage={"R-1","2","M-1","3","builder","1","I-9"};
    r.trajectory_geometry_complete=true;r.constraints={50,120,20,true};
    r.waypoints={{"W-1",{60,25},80,true,AltitudeReference::AGL},
                 {"W-2",{60.01,25.01},80,false,AltitudeReference::AGL}};
    r.segments={{"S-1","W-1","W-2",1244,12,0.2,1.5}};
    RouteSpeedProfile p;p.vehicle_model_id="UAV-1";p.vehicle_model_version="2";
    p.configuration_id="CFG-1";p.speed_reference="TAS";p.dynamics_source="validated";
    p.profile_id="P-1";p.profile_version="1";
    p.segments={{"S-1",1244,10,10,12,100,100,1044,2,2,8,15,8,15}};
    r.speed_profile=p;return r;
}
static RegulatoryComplianceAssessment regulatoryFixture() {
    RegulatoryComplianceAssessment a;a.assessment_id="REG-1";a.mission_id="M-1";a.mission_version="3";
    a.route_id="R-1";a.route_version="2";a.calculation_input_version="I-9";
    a.authority_id="rules-engine";a.rule_set_id="RULES";a.rule_set_version="2026.1";
    a.source_reference="regulatory-rules:1";a.permanent_airspace_snapshot_id="AIP-SNAPSHOT-1";
    a.evaluated_at_epoch_s=1000;a.valid_until_epoch_s=2000;
    a.applicable_rule_count=3;a.evaluated_rule_count=3;a.operation_scope_complete=true;
    a.decision=RegulatoryComplianceDecision::Compliant;return a;
}
static void setupProfile(FlightProfile& p,VehicleCompilationInput& v) {
    p.mission_id="M-1";p.mission_version="3";p.profile_id="P-1";p.profile_version="1";
    p.source_candidate_id="C-1";p.route_id="R-1";p.route_version="2";
    p.calculation_input_version="I-9";p.dependency_identity="DEP-1";p.valid=true;
    p.points={{{0,0},50,0,true},{{0,0.001},60,10,true}};
    v.uav_id="UAV-1";v.configuration_version="CFG-1";v.performance_version="PERF-1";
    v.maximum_altitude_m=120;v.minimum_speed_mps=5;v.maximum_speed_mps=20;v.cruise_speed_mps=12;
}
static MissionLifecycleRecord validatingRecord() {
    MissionLifecycleRecord r;r.mission_id="M-1";r.mission_version="3";
    r.state=MissionLifecycleState::Validating;r.current_route_id="R-1";
    r.current_route_version="2";r.current_calculation_input_version="I-9";return r;
}
static MissionLifecycleEvidence baseEvidence() {
    MissionLifecycleEvidence e;e.mission_id="M-1";e.mission_version="3";e.route_id="R-1";
    e.route_version="2";e.calculation_input_version="I-9";e.vehicle_configuration_id="CFG-1";
    e.operator_review_accepted=true;return e;
}
static void testCoordinatorBuildsTrustedEvidenceButFailsClosedOnMissingServices() {
    auto record=validatingRecord();auto evidence=baseEvidence();auto route=routeFixture();
    FlightProfile profile;VehicleCompilationInput vehicle;setupProfile(profile,vehicle);
    const auto result=MissionLifecycleValidationCoordinator::finalizeReady(
        record,evidence,route,regulatoryFixture(),1500,profile,vehicle,
        "OPERATOR-1","2026-11-15T12:00:00Z","finalize readiness");
    assert(result.finalization.final_integrity_evidence.status==MissionLifecycleValidationStatus::Passed);
    assert(result.finalization.transition.state==MissionLifecycleState::Validating);
    assert(!result.finalization.transition.accepted);
    assert(record.state==MissionLifecycleState::Validating);
    assert(std::find(result.finalization.transition.blockers.begin(),
        result.finalization.transition.blockers.end(),"NOTAM_VALIDATION_EVIDENCE_REQUIRED")
        !=result.finalization.transition.blockers.end());
}
static void testMt01CannotSkipQualityEvidence() {
    auto record=validatingRecord();record.mission_template_ids={"MT-01"};
    auto evidence=baseEvidence();auto route=routeFixture();
    FlightProfile profile;VehicleCompilationInput vehicle;setupProfile(profile,vehicle);
    const auto result=MissionLifecycleValidationCoordinator::finalizeReady(
        record,evidence,route,regulatoryFixture(),1500,profile,vehicle,
        "OPERATOR-1","2026-11-15T12:00:00Z","MT01 readiness");
    assert(!result.mt01_quality_evaluated);
    assert(!result.finalization.transition.accepted);
    assert(std::find(result.finalization.transition.blockers.begin(),
        result.finalization.transition.blockers.end(),"TASK_QUALITY_VALIDATION_EVIDENCE_REQUIRED")
        !=result.finalization.transition.blockers.end());
}
static void testAuthoritativeSourcePathRejectsMissingServicesBeforeFinalization() {
    auto record=validatingRecord();
    auto evidence=baseEvidence();
    MissionLifecycleAuthoritativeSources sources;
    FlightProfile profile;
    VehicleCompilationInput vehicle;
    const auto result=MissionLifecycleValidationCoordinator::finalizeReadyFromAuthoritativeSources(
        record,evidence,sources,regulatoryFixture(),1500,profile,vehicle,
        "OPERATOR-1","2026-11-16T00:00:00Z","authoritative readiness");
    assert(!result.finalization.transition.accepted);
    assert(record.state==MissionLifecycleState::Validating);
    assert(std::find(result.evidence_assembly_blockers.begin(),
        result.evidence_assembly_blockers.end(),"AUTHORITATIVE_PLANNING_DECISION_INCOMPLETE")
        !=result.evidence_assembly_blockers.end());
    assert(std::find(result.evidence_assembly_blockers.begin(),
        result.evidence_assembly_blockers.end(),"NOTAM_SNAPSHOT_REQUIRED")
        !=result.evidence_assembly_blockers.end());
    assert(std::find(result.evidence_assembly_blockers.begin(),
        result.evidence_assembly_blockers.end(),"PERMANENT_AIRSPACE_VALIDATION_SOURCE_REQUIRED")
        !=result.evidence_assembly_blockers.end());
    assert(std::find(result.evidence_assembly_blockers.begin(),
        result.evidence_assembly_blockers.end(),"OBSTACLE_CLEARANCE_VALIDATION_SOURCE_REQUIRED")
        !=result.evidence_assembly_blockers.end());
    assert(std::find(result.evidence_assembly_blockers.begin(),
        result.evidence_assembly_blockers.end(),"TERRAIN_VALIDATION_SOURCE_REQUIRED")
        !=result.evidence_assembly_blockers.end());
    assert(std::find(result.evidence_assembly_blockers.begin(),
        result.evidence_assembly_blockers.end(),"WEATHER_ENERGY_VALIDATION_SOURCE_REQUIRED")
        !=result.evidence_assembly_blockers.end());
}
int main(){
    testCoordinatorBuildsTrustedEvidenceButFailsClosedOnMissingServices();
    testMt01CannotSkipQualityEvidence();
    testAuthoritativeSourcePathRejectsMissingServicesBeforeFinalization();
    return 0;
}
