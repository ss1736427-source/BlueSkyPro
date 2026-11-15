#pragma once
#include "mission_lifecycle.hpp"
#include "model/route_model.hpp"
#include <algorithm>
#include <cmath>
namespace bluesky::planning {
inline MissionLifecycleValidationEvidence makeTrajectoryGeometryValidationEvidence(const Route& route) {
    MissionLifecycleValidationEvidence e;
    e.mission_id=route.lineage.mission_id; e.mission_version=route.lineage.mission_version;
    e.route_id=route.lineage.route_id; e.route_version=route.lineage.route_version;
    e.calculation_input_version=route.lineage.calculation_input_version;
    e.validator_id="canonical-trajectory-geometry-validator"; e.validator_version="1";
    e.source_reference="canonical-route:"+route.lineage.route_id+"@"+route.lineage.route_version;
    const bool identity=!route.lineage.mission_id.empty()&&!route.lineage.mission_version.empty()
        &&!route.lineage.route_id.empty()&&!route.lineage.route_version.empty()
        &&!route.lineage.calculation_input_version.empty()&&!route.lineage.generator_id.empty()
        &&!route.lineage.generator_version.empty();
    bool valid=route.trajectory_geometry_complete&&route.waypoints.size()>=2
        &&route.segments.size()+1==route.waypoints.size()&&route.speed_profile.has_value();
    if(valid) {
        const auto& p=*route.speed_profile;
        valid=!p.profile_id.empty()&&!p.profile_version.empty()&&!p.vehicle_model_id.empty()
            &&!p.vehicle_model_version.empty()&&!p.configuration_id.empty()
            &&!p.speed_reference.empty()&&!p.dynamics_source.empty()
            &&p.segments.size()==route.segments.size();
        for(std::size_t i=0;valid&&i<route.segments.size();++i) {
            const auto& s=route.segments[i]; const auto& v=p.segments[i];
            const double tol=std::max(0.5,s.distance_m*0.01);
            const double partition=v.acceleration_distance_m+v.deceleration_distance_m+v.cruise_distance_m;
            valid=!s.segment_id.empty()&&s.from_waypoint_id==route.waypoints[i].waypoint_id
                &&s.to_waypoint_id==route.waypoints[i+1].waypoint_id
                &&std::isfinite(s.distance_m)&&s.distance_m>0
                &&std::isfinite(s.nominal_speed_mps)&&s.nominal_speed_mps>0
                &&v.segment_id==s.segment_id&&std::isfinite(v.distance_m)
                &&std::abs(v.distance_m-s.distance_m)<=tol
                &&std::isfinite(v.entry_speed_mps)&&v.entry_speed_mps>0
                &&std::isfinite(v.exit_speed_mps)&&v.exit_speed_mps>0
                &&std::isfinite(v.maximum_reachable_speed_mps)&&v.maximum_reachable_speed_mps>0
                &&std::isfinite(v.minimum_speed_mps)&&v.minimum_speed_mps>0
                &&std::isfinite(v.maximum_speed_mps)&&v.maximum_speed_mps>=v.minimum_speed_mps
                &&std::isfinite(v.applied_minimum_speed_mps)&&v.applied_minimum_speed_mps>0
                &&std::isfinite(v.applied_maximum_speed_mps)&&v.applied_maximum_speed_mps>=v.applied_minimum_speed_mps
                &&v.entry_speed_mps+1e-6>=v.applied_minimum_speed_mps
                &&v.entry_speed_mps<=v.applied_maximum_speed_mps+1e-6
                &&v.exit_speed_mps+1e-6>=v.applied_minimum_speed_mps
                &&v.exit_speed_mps<=v.applied_maximum_speed_mps+1e-6
                &&std::isfinite(v.acceleration_distance_m)&&v.acceleration_distance_m>=0
                &&std::isfinite(v.deceleration_distance_m)&&v.deceleration_distance_m>=0
                &&std::isfinite(v.cruise_distance_m)&&v.cruise_distance_m>=0
                &&std::isfinite(partition)&&std::abs(partition-s.distance_m)<=tol
                &&std::isfinite(v.maximum_acceleration_mps2)&&v.maximum_acceleration_mps2>0
                &&std::isfinite(v.maximum_deceleration_mps2)&&v.maximum_deceleration_mps2>0;
            if(s.bank_angle_rad) valid=valid&&std::isfinite(*s.bank_angle_rad)&&*s.bank_angle_rad>=0;
            if(s.lateral_acceleration_mps2) valid=valid&&std::isfinite(*s.lateral_acceleration_mps2)&&*s.lateral_acceleration_mps2>=0;
        }
    }
    e.result_id=e.validator_id+":"+route.lineage.route_id+"@"+route.lineage.route_version+":"+route.lineage.calculation_input_version;
    e.status=identity&&valid?MissionLifecycleValidationStatus::Passed:MissionLifecycleValidationStatus::Failed;
    return e;
}
} // namespace bluesky::planning
