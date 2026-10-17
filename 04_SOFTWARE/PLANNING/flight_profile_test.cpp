#include "flight_profile.hpp"
#include <cassert>
#include <cmath>
#include <limits>
using namespace bluesky::planning;
int main() {
    SelectedRouteSet selected;
    selected.mission_id="M-1"; selected.mission_version="7"; selected.candidate_id="C-2";
    selected.source_dependency_identity="DEP";
    selected.calculation_input_version="ENV-3";
    selected.route_elements={"WP1","WP2"};
    Route route;
    route.lineage.route_id="R-1"; route.lineage.route_version="4";
    route.lineage.mission_id="M-1"; route.lineage.mission_version="7";
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

    auto stale_selected_mission = selected;
    stale_selected_mission.mission_id = "M-STALE";
    const auto stale_mission_profile = FlightProfileBuilder::build(
        stale_selected_mission,route,trajectory,"FP-STALE-MISSION","1.0.0");
    assert(!stale_mission_profile.valid);
    assert(stale_mission_profile.points.empty());

    auto stale_selected_environment = selected;
    stale_selected_environment.calculation_input_version = "ENV-STALE";
    const auto stale_environment_profile = FlightProfileBuilder::build(
        stale_selected_environment,route,trajectory,"FP-STALE-ENV","1.0.0");
    assert(!stale_environment_profile.valid);
    assert(stale_environment_profile.points.empty());

    auto wrong_selected_order = selected;
    wrong_selected_order.route_elements[1] = "WP-STALE";
    const auto wrong_selected_route_profile = FlightProfileBuilder::build(
        wrong_selected_order,route,trajectory,"FP-STALE-ROUTE","1.0.0");
    assert(!wrong_selected_route_profile.valid);
    assert(wrong_selected_route_profile.points.empty());

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

    auto non_finite_geometry=trajectory;
    non_finite_geometry.points[1].position.latitude_deg=
        std::numeric_limits<double>::quiet_NaN();
    const auto non_finite_invalid=FlightProfileBuilder::build(
        selected,route,non_finite_geometry,"FP-5","1.0.0");
    assert(!non_finite_invalid.valid);
    assert(non_finite_invalid.points.empty());

    // Segment durations must match the differences between cumulative point times.
    Route multi_segment_route = route;
    multi_segment_route.waypoints.push_back({"WP3",{0.0,0.002},120.0,true});
    SelectedRouteSet multi_segment_selected = selected;
    multi_segment_selected.route_elements.push_back("WP3");
    multi_segment_route.segments.push_back({"S2","WP2","WP3",111.32,10.0});
    TrajectoryResult inconsistent_segment_time = trajectory;
    inconsistent_segment_time.points.push_back(
        {"WP3",{0.0,0.002},120.0,12.5,3.0});
    inconsistent_segment_time.segments[0].traversal_time_s = 8.0;
    inconsistent_segment_time.segments.push_back({"S2",8.9,90.0,4.5,1.0});
    inconsistent_segment_time.total_time_s = 12.5;
    const auto inconsistent_time_profile = FlightProfileBuilder::build(
        multi_segment_selected,multi_segment_route,inconsistent_segment_time,"FP-6","1.0.0");
    assert(!inconsistent_time_profile.valid);
    assert(inconsistent_time_profile.points.empty());
    return 0;
}
