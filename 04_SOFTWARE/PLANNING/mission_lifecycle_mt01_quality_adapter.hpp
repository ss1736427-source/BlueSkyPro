#pragma once
#include "coverage_path_planner.hpp"
#include "mission_lifecycle.hpp"
#include "mt01_kinematic_geometry_builder.hpp"
#include <algorithm>
#include <cmath>
namespace bluesky::planning {
inline MissionLifecycleValidationEvidence makeMt01TaskQualityValidationEvidence(
    const Route& route,const CoverageMissionInput& input,const CoveragePlanResult& coverage,
    const Mt01KinematicGeometryResult& geometry) {
    MissionLifecycleValidationEvidence e;
    e.mission_id=route.lineage.mission_id; e.mission_version=route.lineage.mission_version;
    e.route_id=route.lineage.route_id; e.route_version=route.lineage.route_version;
    e.calculation_input_version=route.lineage.calculation_input_version;
    e.validator_id="mt01-camera-coverage-quality-validator"; e.validator_version="1";
    e.source_reference="coverage-plan:"+input.mission_id+"@"+input.mission_version
        +";route:"+route.lineage.route_id+"@"+route.lineage.route_version;
    const auto positive=[](double x){return std::isfinite(x)&&x>0.0;};
    const auto close=[](double a,double b,double t){return std::isfinite(a)&&std::isfinite(b)&&std::abs(a-b)<=t;};
    const auto& c=input.camera; const auto& m=coverage.metrics;
    bool ok=input.mission_id==route.lineage.mission_id&&input.mission_version==route.lineage.mission_version
        &&!route.lineage.route_id.empty()&&!route.lineage.route_version.empty()
        &&!route.lineage.calculation_input_version.empty()&&coverage.valid&&geometry.feasible
        &&!c.camera_configuration_id.empty()&&!c.calibration_source_reference.empty()
        &&c.intrinsic_calibration_verified&&coverage.intrinsic_calibration_verified
        &&m.coverage_line_count>0&&m.coverage_segment_count>0&&m.exclusion_intersection_count==0
        &&!coverage.capture_points.empty()&&geometry.capture_point_waypoint_indices.size()==coverage.capture_points.size()
        &&geometry.waypoints.size()>=2&&geometry.segments.size()+1==geometry.waypoints.size()
        &&route.trajectory_geometry_complete&&route.waypoints.size()==geometry.waypoints.size()
        &&route.segments.size()==geometry.segments.size()&&route.speed_profile.has_value()
        &&route.speed_profile->camera_configuration_id==c.camera_configuration_id
        &&positive(c.target_gsd_cm_px)&&positive(m.target_gsd_cm_px)
        &&close(m.target_gsd_cm_px,c.target_gsd_cm_px,1e-6)
        &&std::isfinite(c.forward_overlap_percent)&&c.forward_overlap_percent>=0&&c.forward_overlap_percent<100
        &&std::isfinite(c.side_overlap_percent)&&c.side_overlap_percent>=0&&c.side_overlap_percent<100
        &&positive(m.calculated_altitude_agl_m)&&m.calculated_altitude_agl_m>=input.minimum_altitude_agl_m
        &&m.calculated_altitude_agl_m<=input.maximum_altitude_agl_m
        &&positive(m.footprint_width_m)&&positive(m.footprint_height_m)&&positive(m.line_spacing_m)
        &&positive(m.forward_trigger_spacing_m)&&positive(m.estimated_coverage_leg_length_m)
        &&positive(m.estimated_coverage_leg_time_s)&&positive(m.nominal_trigger_interval_s)
        &&positive(m.maximum_camera_ground_speed_mps)&&std::isfinite(m.nominal_motion_blur_px)
        &&m.nominal_motion_blur_px>=0&&positive(input.cruise_speed_mps)
        &&positive(c.focal_length_mm)&&positive(c.pixel_pitch_um)&&c.image_width_px>0&&c.image_height_px>0;
    if(ok) ok=geometry.vehicle_model_id==route.speed_profile->vehicle_model_id
        &&geometry.vehicle_model_version==route.speed_profile->vehicle_model_version
        &&geometry.vehicle_configuration_id==route.speed_profile->configuration_id;
    if(ok) ok=geometry.vehicle_model_id==route.speed_profile->vehicle_model_id
        &&geometry.vehicle_model_version==route.speed_profile->vehicle_model_version
        &&geometry.vehicle_configuration_id==route.speed_profile->configuration_id;
    for(std::size_t i=0;ok&&i<geometry.waypoints.size();++i) {
        const auto& a=geometry.waypoints[i]; const auto& b=route.waypoints[i].position;
        ok=close(a.latitude_deg,b.latitude_deg,1e-8)&&close(a.longitude_deg,b.longitude_deg,1e-8);
    }
    for(std::size_t i=0;ok&&i<geometry.segments.size();++i) {
        const auto& generated=geometry.segments[i]; const auto& canonical=route.segments[i];
        ok=generated.segment_id==canonical.segment_id
            &&generated.from_waypoint_index==i&&generated.to_waypoint_index==i+1
            &&close(generated.distance_m,canonical.distance_m,std::max(0.5,canonical.distance_m*0.01))
            &&close(generated.nominal_speed_mps,canonical.nominal_speed_mps,1e-6);
    }
    for(std::size_t i=0;ok&&i<geometry.segments.size();++i) {
        const auto& generated=geometry.segments[i]; const auto& canonical=route.segments[i];
        ok=generated.segment_id==canonical.segment_id
            &&generated.from_waypoint_index==i&&generated.to_waypoint_index==i+1
            &&close(generated.distance_m,canonical.distance_m,std::max(0.5,canonical.distance_m*0.01))
            &&close(generated.nominal_speed_mps,canonical.nominal_speed_mps,1e-6);
    }
    for(std::size_t i=0;ok&&i<geometry.capture_point_waypoint_indices.size();++i) {
        const auto index=geometry.capture_point_waypoint_indices[i];
        if(index>=geometry.waypoints.size()){ok=false;break;}
        const auto& a=coverage.capture_points[i].position; const auto& b=geometry.waypoints[index];
        ok=close(a.latitude_deg,b.latitude_deg,1e-8)&&close(a.longitude_deg,b.longitude_deg,1e-8);
    }
    e.result_id=e.validator_id+":"+route.lineage.route_id+"@"+route.lineage.route_version
        +":"+c.camera_configuration_id+":"+std::to_string(coverage.capture_points.size());
    e.status=ok?MissionLifecycleValidationStatus::Passed:MissionLifecycleValidationStatus::Failed;
    return e;
}
} // namespace bluesky::planning
