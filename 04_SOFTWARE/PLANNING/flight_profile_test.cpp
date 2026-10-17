#include "flight_profile.hpp"
#include <cassert>
#include <cmath>
using namespace bluesky::planning;
int main() {
    SelectedRouteSet selected;
    selected.mission_id="M-1"; selected.mission_version="7"; selected.candidate_id="C-2";
    selected.source_dependency_identity="DEP";
    Route route;
    route.lineage.route_id="R-1"; route.lineage.route_version="4";
    route.lineage.calculation_input_version="ENV-3";
    route.waypoints={
        {"WP1",{0.0,0.0},10.0,true},
        {"WP2",{0.0,0.001},100.0,true}
    };
    route.segments={{"S1","WP1","WP2",111.32,10.0}};
    TrajectoryResult trajectory;
    trajectory.status=TrajectoryStatus::Feasible;
    trajectory.route_id="R-1"; trajectory.route_version="4";
    trajectory.points={
        {"WP1",{0.0,0.0},10.0,0.0,0.0},
        {"WP2",{0.0,0.001},100.0,12.5,2.0}
    };
    trajectory.segments={{"S1",8.9,90.0,12.5,2.0}};
    trajectory.total_time_s=12.5;

    const auto p=FlightProfileBuilder::build(selected,route,trajectory,"FP-1","1.0.0");
    assert(p.valid);
    assert(p.profile_id=="FP-1");
    assert(p.profile_version=="1.0.0");
    assert(p.points.size()==2);
    assert(p.points[1].altitude_m==100.0);
    assert(p.points[0].cumulative_time_s==0.0);
    assert(std::abs(p.points[1].cumulative_time_s-12.5)<1e-9);
    assert(p.dependency_identity=="DEP|R-1|4|ENV-3");

    auto wrong_version=trajectory;
    wrong_version.route_version="stale";
    const auto invalid=FlightProfileBuilder::build(selected,route,wrong_version,"FP-2","1.0.0");
    assert(!invalid.valid);
    assert(invalid.points.empty());

    auto wrong_geometry=trajectory;
    wrong_geometry.points[1].position.longitude_deg=0.002;
    const auto geometry_invalid=FlightProfileBuilder::build(
        selected,route,wrong_geometry,"FP-3","1.0.0");
    assert(!geometry_invalid.valid);
    assert(geometry_invalid.points.empty());

    auto wrong_segment=trajectory;
    wrong_segment.segments[0].segment_id="STALE-SEGMENT";
    const auto segment_invalid=FlightProfileBuilder::build(
        selected,route,wrong_segment,"FP-4","1.0.0");
    assert(!segment_invalid.valid);
    assert(segment_invalid.points.empty());
    return 0;
}
