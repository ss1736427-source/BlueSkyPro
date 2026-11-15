#include "mission_lifecycle_trajectory_geometry_adapter.hpp"
#include "mission_lifecycle_mt01_quality_adapter.hpp"
#include "mission_lifecycle_regulatory_adapter.hpp"
#include <cassert>
using namespace bluesky::planning;
static Route routeFixture() {
    Route r;r.lineage={"R-1","2","M-1","3","builder","1","I-9"};r.trajectory_geometry_complete=true;
    r.waypoints={{"W-1",{60,25},100,true,AltitudeReference::AGL},{"W-2",{60.01,25.01},100,false,AltitudeReference::AGL}};
    r.segments={{"S-1","W-1","W-2",1244,12,0.2,1.5}};
    RouteSpeedProfile p;p.vehicle_model_id="UAV-1";p.vehicle_model_version="2";p.configuration_id="CFG-1";
    p.speed_reference="TAS";p.dynamics_source="validated";p.profile_id="P-1";p.profile_version="1";
    p.segments={{"S-1",1244,10,10,12,100,100,1044,2,2,8,15,8,15}};r.speed_profile=p;return r;
}
static void geometryTests() {
    auto r=routeFixture();assert(makeTrajectoryGeometryValidationEvidence(r).status==MissionLifecycleValidationStatus::Passed);
    r.trajectory_geometry_complete=false;assert(makeTrajectoryGeometryValidationEvidence(r).status==MissionLifecycleValidationStatus::Failed);
    r=routeFixture();r.speed_profile->segments[0].segment_id="STALE";
    assert(makeTrajectoryGeometryValidationEvidence(r).status==MissionLifecycleValidationStatus::Failed);
    r=routeFixture();r.speed_profile->segments[0].cruise_distance_m=9000;
    assert(makeTrajectoryGeometryValidationEvidence(r).status==MissionLifecycleValidationStatus::Failed);
}
static void qualityTests() {
    auto r=routeFixture();r.speed_profile->camera_configuration_id="CAM-1";
    CoverageMissionInput in;in.mission_id="M-1";in.mission_version="3";in.minimum_altitude_agl_m=50;in.maximum_altitude_agl_m=150;
    in.cruise_speed_mps=10;in.camera.focal_length_mm=24;in.camera.pixel_pitch_um=4;in.camera.image_width_px=6000;
    in.camera.image_height_px=4000;in.camera.target_gsd_cm_px=2;in.camera.forward_overlap_percent=80;
    in.camera.side_overlap_percent=70;in.camera.camera_configuration_id="CAM-1";
    in.camera.calibration_source_reference="calibration-report";in.camera.intrinsic_calibration_verified=true;
    CoveragePlanResult c;c.valid=true;c.intrinsic_calibration_verified=true;c.capture_points={{{60,25},0,0}};
    c.metrics.target_gsd_cm_px=2;c.metrics.calculated_altitude_agl_m=100;c.metrics.footprint_width_m=120;
    c.metrics.footprint_height_m=80;c.metrics.line_spacing_m=36;c.metrics.forward_trigger_spacing_m=16;
    c.metrics.estimated_coverage_leg_length_m=1000;c.metrics.estimated_coverage_leg_time_s=100;
    c.metrics.nominal_trigger_interval_s=1.6;c.metrics.maximum_camera_ground_speed_mps=15;
    c.metrics.coverage_line_count=2;c.metrics.coverage_segment_count=2;
    Mt01KinematicGeometryResult g;g.feasible=true;g.waypoints={{60,25},{60.01,25.01}};
    g.capture_point_waypoint_indices={0};g.segments.resize(1);
    assert(makeMt01TaskQualityValidationEvidence(r,in,c,g).status==MissionLifecycleValidationStatus::Passed);
    in.camera.intrinsic_calibration_verified=false;
    assert(makeMt01TaskQualityValidationEvidence(r,in,c,g).status==MissionLifecycleValidationStatus::Failed);
    in.camera.intrinsic_calibration_verified=true;c.metrics.exclusion_intersection_count=1;
    assert(makeMt01TaskQualityValidationEvidence(r,in,c,g).status==MissionLifecycleValidationStatus::Failed);
    c.metrics.exclusion_intersection_count=0;g.capture_point_waypoint_indices={99};
    assert(makeMt01TaskQualityValidationEvidence(r,in,c,g).status==MissionLifecycleValidationStatus::Failed);
}
static void regulatoryTests() {
    RegulatoryComplianceAssessment a;a.assessment_id="A-1";a.mission_id="M-1";a.mission_version="3";
    a.route_id="R-1";a.route_version="2";a.calculation_input_version="I-9";
    a.authority_id="rules-engine";a.rule_set_id="RULES";a.rule_set_version="2026.1";
    a.source_reference="snapshot:1";a.evaluated_at_epoch_s=1000;a.valid_until_epoch_s=2000;
    a.applicable_rule_count=12;a.evaluated_rule_count=12;a.operation_scope_complete=true;
    a.decision=RegulatoryComplianceDecision::Compliant;
    assert(makeRegulatoryValidationEvidence(a,1500).status==MissionLifecycleValidationStatus::Passed);
    assert(makeRegulatoryValidationEvidence(a,2001).status==MissionLifecycleValidationStatus::Failed);
    a.evaluated_rule_count=10;assert(makeRegulatoryValidationEvidence(a,1500).status==MissionLifecycleValidationStatus::Failed);
    a.evaluated_rule_count=12;a.unresolved_findings.push_back("RESTRICTION");
    assert(makeRegulatoryValidationEvidence(a,1500).status==MissionLifecycleValidationStatus::Failed);
}
int main(){geometryTests();qualityTests();regulatoryTests();return 0;}
