#include "flight_profile.hpp"
#include <cassert>
using namespace bluesky::planning;
int main() {
    SelectedRouteSet selected;
    selected.mission_id="M-1"; selected.mission_version="7"; selected.candidate_id="C-2";
    selected.source_dependency_identity="DEP";
    Route route;
    route.lineage.route_id="R-1"; route.lineage.route_version="4";
    route.lineage.calculation_input_version="ENV-3";

    route.waypoints.push_back({ "WP1", {0.0,0.0}, 10.0, true });
    route.waypoints.push_back({ "WP2", {0.0,0.001}, 100.0, true });
    const auto p=FlightProfileBuilder::build(selected,route,"FP-1","1.0.0");
    assert(p.profile_id=="FP-1");
    assert(p.points.size()==2);
    assert(p.points[1].altitude_m==100.0);
    assert(p.dependency_identity=="DEP|R-1|4|ENV-3");
    return 0;
}
