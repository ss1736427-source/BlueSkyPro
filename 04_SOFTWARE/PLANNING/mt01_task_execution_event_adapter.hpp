#pragma once
#include "coverage_path_planner.hpp"
#include "mt01_kinematic_geometry_builder.hpp"
#include "model/route_model.hpp"
#include <cmath>
#include <cstddef>
#include <string>
#include <vector>
namespace bluesky::planning {
struct Mt01TaskExecutionEvent {
 std::size_t capture_point_index{0}, coverage_segment_index{0}, route_waypoint_index{0};
 double duration_s{0.0}, energy_wh{0.0};
 std::string mission_id, mission_version, route_id, route_version, vehicle_id, vehicle_configuration_version;
 std::string equipment_id, equipment_profile_id, calibration_reference, source_reference, source_version, method_version;
};
struct Mt01TaskExecutionEventValidation {
 bool valid{false}; double total_duration_s{0.0}, total_energy_wh{0.0}; std::vector<std::string> findings;
};
class Mt01TaskExecutionEventAdapter {
public:
 static Mt01TaskExecutionEventValidation apply(Route& route,const CoveragePlanResult& coverage,
  const Mt01KinematicGeometryResult& geometry,const std::vector<Mt01TaskExecutionEvent>& events,
  const std::string& vehicleId,const std::string& vehicleConfigurationVersion,
  const std::string& equipmentId,const std::string& equipmentProfileId,const std::string& calibrationReference) {
  Mt01TaskExecutionEventValidation result;
  const auto fail=[&](const std::string& reason){result.valid=false;result.findings.push_back(reason);return result;};
  if(!coverage.valid||geometry.capture_point_waypoint_indices.size()!=coverage.capture_points.size()
   ||route.waypoints.size()!=geometry.waypoints.size()||route.lineage.mission_id.empty()
   ||route.lineage.mission_version.empty()||route.lineage.route_id.empty()||route.lineage.route_version.empty()
   ||vehicleId.empty()||vehicleConfigurationVersion.empty()||equipmentId.empty()||equipmentProfileId.empty()
   ||calibrationReference.empty())
   return fail("MT01_TASK_EVENT_BINDING_INCOMPLETE: canonical route, capture mapping and assigned equipment/calibration identity are mandatory.");
  if(events.size()!=coverage.capture_points.size())
   return fail("MT01_TASK_EVENT_COUNT_MISMATCH: exactly one validated execution event is required for every capture point.");
  std::vector<bool> seen(coverage.capture_points.size(),false);
  for(const auto& event:events) {
   if(event.capture_point_index>=coverage.capture_points.size()||seen[event.capture_point_index])
    return fail("MT01_TASK_EVENT_DUPLICATE_OR_UNKNOWN_CAPTURE: event capture indices must be unique and in range.");
   seen[event.capture_point_index]=true;
   const auto expectedWaypoint=geometry.capture_point_waypoint_indices[event.capture_point_index];
   const auto& capture=coverage.capture_points[event.capture_point_index];
   if(event.coverage_segment_index!=capture.coverage_segment_index||event.route_waypoint_index!=expectedWaypoint
    ||expectedWaypoint>=route.waypoints.size())
    return fail("MT01_TASK_EVENT_ROUTE_MAPPING_MISMATCH: event segment and waypoint must match the canonical capture mapping.");
   const auto& waypoint=route.waypoints[expectedWaypoint];
   if(std::abs(waypoint.position.latitude_deg-capture.position.latitude_deg)>1e-9
    ||std::abs(waypoint.position.longitude_deg-capture.position.longitude_deg)>1e-9)
    return fail("MT01_TASK_EVENT_POSITION_MISMATCH: mapped route waypoint does not coincide with its capture point.");
   if(!std::isfinite(event.duration_s)||event.duration_s<0.0||!std::isfinite(event.energy_wh)||event.energy_wh<0.0)
    return fail("MT01_TASK_EVENT_COST_INVALID: duration and energy must be finite and non-negative.");
   if(event.mission_id!=route.lineage.mission_id||event.mission_version!=route.lineage.mission_version
    ||event.route_id!=route.lineage.route_id||event.route_version!=route.lineage.route_version
    ||event.vehicle_id!=vehicleId||event.vehicle_configuration_version!=vehicleConfigurationVersion
    ||event.equipment_id!=equipmentId||event.equipment_profile_id!=equipmentProfileId
    ||event.calibration_reference!=calibrationReference||event.source_reference.empty()
    ||event.source_version.empty()||event.method_version.empty())
    return fail("MT01_TASK_EVENT_PROVENANCE_MISMATCH: event source, method, mission, route, vehicle and equipment lineage must match exactly.");
   if(waypoint.task_execution_duration_s||waypoint.task_execution_energy_wh)
    return fail("MT01_TASK_EVENT_ALREADY_APPLIED: route waypoint already contains task execution costs.");
   result.total_duration_s+=event.duration_s; result.total_energy_wh+=event.energy_wh;
  }
  for(bool present:seen) if(!present) return fail("MT01_TASK_EVENT_MISSING_CAPTURE: every capture point must have one event.");
  for(const auto& event:events) {
   auto& waypoint=route.waypoints[event.route_waypoint_index];
   waypoint.task_execution_duration_s=event.duration_s;
   waypoint.task_execution_energy_wh=event.energy_wh;
  }
  result.valid=true; return result;
 }
};
} // namespace bluesky::planning
