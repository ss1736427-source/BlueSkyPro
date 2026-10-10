#include "vertical_route_builder.hpp"
#include <cmath>

namespace bluesky::planning {
namespace {
double distance_m(const GeoPoint&a,const GeoPoint&b) {
    constexpr double k=111320.0;
    const double lat=(a.latitude_deg+b.latitude_deg)*0.5*3.141592653589793/180.0;
    const double dx=(b.longitude_deg-a.longitude_deg)*k*std::cos(lat);
    const double dy=(b.latitude_deg-a.latitude_deg)*k;
    return std::hypot(dx,dy);
}
RouteWaypoint wp(const char* id,const GeoPoint&p,double alt,bool mandatory) {
    return {id,p,alt,mandatory};
}
}
VerticalRouteBuildResult VerticalRouteBuilder::build(
    const VerticalRouteBuildInput& input,const std::string& calculation_version) {
    VerticalRouteBuildResult r;
    r.calculation_version=calculation_version;
    r.launch_profile=VerticalLaunchRecoveryPlanner::calculate(input.launch_profile,calculation_version);
    r.recovery_profile=VerticalLaunchRecoveryPlanner::calculate(input.recovery_profile,calculation_version);
    if(r.launch_profile.status!=VerticalProfileStatus::Valid ||
       r.recovery_profile.status!=VerticalProfileStatus::Valid) return r;

    if(input.cruise_altitude_m < r.launch_profile.safe_transition_altitude_m ||
       input.cruise_altitude_m < r.recovery_profile.safe_transition_altitude_m)
        return r;

    r.route.lineage.route_id=input.route_id;
    r.route.lineage.route_version=input.route_version;
    r.route.lineage.mission_id=input.mission_id;
    r.route.lineage.mission_version=input.mission_version;
    r.route.lineage.generator_id="vertical_route_builder";
    r.route.lineage.generator_version="1.0.0";
    r.route.lineage.calculation_input_version=calculation_version;

    r.route.waypoints.push_back(wp("LAUNCH",input.launch,0.0,true));
    r.route.waypoints.push_back(wp("LAUNCH_TRANSITION",input.launch,r.launch_profile.safe_transition_altitude_m,true));
    r.route.waypoints.push_back(wp("CRUISE_START",input.launch,input.cruise_altitude_m,true));
    r.route.waypoints.push_back(wp("CRUISE_END",input.recovery,input.cruise_altitude_m,true));
    r.route.waypoints.push_back(wp("RECOVERY_TRANSITION",input.recovery,r.recovery_profile.safe_transition_altitude_m,true));
    r.route.waypoints.push_back(wp("RECOVERY",input.recovery,0.0,true));

    for(size_t i=1;i<r.route.waypoints.size();++i) {
        const auto& a=r.route.waypoints[i-1]; const auto& b=r.route.waypoints[i];
        RouteSegment s;
        s.segment_id="SEG-"+std::to_string(i);
        s.from_waypoint_id=a.waypoint_id; s.to_waypoint_id=b.waypoint_id;
        s.distance_m=distance_m(a.position,b.position);
        s.nominal_speed_mps=input.transition_speed_mps;
        r.route.segments.push_back(s);
    }
    r.feasible=true;
    return r;
}
} // namespace bluesky::planning
