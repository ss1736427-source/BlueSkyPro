#include "vertical_route_builder.hpp"
#include <cassert>
using namespace bluesky::planning;
int main(){
 VerticalRouteBuildInput i;
 i.route_id="R1";i.route_version="1";i.mission_id="M1";i.mission_version="1";
 i.launch={59.0,30.0};i.recovery={59.01,30.01};i.cruise_altitude_m=150;i.transition_speed_mps=10;
 i.launch_profile.point=i.launch;i.launch_profile.terrain_elevation_m=20;i.launch_profile.minimum_clearance_m=30;i.launch_profile.vehicle_minimum_transition_altitude_m=60;i.launch_profile.vehicle_max_altitude_m=200;
 i.recovery_profile=i.launch_profile;i.recovery_profile.point=i.recovery;
 auto r=VerticalRouteBuilder::build(i,"VRT-1");
 assert(r.feasible);assert(r.route.waypoints.size()==6);assert(r.route.segments.size()==5);
 assert(r.route.waypoints[1].altitude_m==50.0);assert(r.route.waypoints[4].altitude_m==50.0);
 i.cruise_altitude_m=40;auto bad=VerticalRouteBuilder::build(i,"VRT-1");assert(!bad.feasible);
 return 0;
}