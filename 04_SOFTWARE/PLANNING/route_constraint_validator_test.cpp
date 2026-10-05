#include "route_constraint_validator.hpp"
#include <cassert>
using namespace bluesky::planning;
static Route base(){ Route r; r.lineage={"R-1","1","M-1","1","generator","1","INPUT-1"}; r.constraints={80,120,30,true}; r.waypoints={{"WP-1",{60,25},100,true},{"WP-2",{60.1,25.1},100,false}}; r.segments={{"S-1","WP-1","WP-2",1000,20}}; return r; }
int main(){
 auto ok=RouteConstraintValidator::validate(base()); assert(ok.status==RouteValidationStatus::Allowed&&ok.findings.empty());
 auto low=base(); low.waypoints[0].altitude_m=50; assert(RouteConstraintValidator::validate(low).status==RouteValidationStatus::Rejected);
 auto high=base(); high.waypoints[1].altitude_m=130; assert(RouteConstraintValidator::validate(high).status==RouteValidationStatus::Rejected);
 auto broken=base(); broken.segments[0].to_waypoint_id="MISSING"; assert(RouteConstraintValidator::validate(broken).status==RouteValidationStatus::Rejected);
 auto disconnected=base(); disconnected.waypoints.push_back({"WP-3",{60.2,25.2},100,false}); assert(RouteConstraintValidator::validate(disconnected).status==RouteValidationStatus::Rejected);
 return 0;
}