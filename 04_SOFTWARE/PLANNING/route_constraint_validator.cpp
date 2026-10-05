#include "route_constraint_validator.hpp"
#include <unordered_map>
namespace bluesky::planning {
RouteValidationResult RouteConstraintValidator::validate(const Route& route) {
 RouteValidationResult result;
 result.validator_id="canonical-route-constraint-validator"; result.validator_version="1";
 result.calculation_input_version=route.lineage.calculation_input_version;
 std::unordered_map<std::string,bool> ids;
 for(const auto& w: route.waypoints){
  ids[w.waypoint_id]=true;
  if(w.altitude_m < route.constraints.minimum_altitude_m)
   result.findings.push_back({RouteFindingCode::AltitudeBelowMinimum,w.waypoint_id,"waypoint altitude below route minimum"});
  if(route.constraints.maximum_altitude_m>0.0 && w.altitude_m>route.constraints.maximum_altitude_m)
   result.findings.push_back({RouteFindingCode::AltitudeAboveMaximum,w.waypoint_id,"waypoint altitude above route maximum"});
 }
 for(const auto& s: route.segments){
  if(!ids.count(s.from_waypoint_id)) result.findings.push_back({RouteFindingCode::MissingWaypointReference,s.segment_id,"segment source waypoint not found"});
  if(!ids.count(s.to_waypoint_id)) result.findings.push_back({RouteFindingCode::MissingWaypointReference,s.segment_id,"segment destination waypoint not found"});
 }
 if(route.waypoints.size()>1) for(const auto& w: route.waypoints){
  bool connected=false; for(const auto& s: route.segments) if(s.from_waypoint_id==w.waypoint_id||s.to_waypoint_id==w.waypoint_id){connected=true;break;}
  if(!connected) result.findings.push_back({RouteFindingCode::SegmentEndpointMismatch,w.waypoint_id,"waypoint is not connected to a route segment"});
 }
 if(!result.findings.empty()) result.status=RouteValidationStatus::Rejected;
 return result;
}
}