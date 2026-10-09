#include "canonical_route_builder.hpp"

#include <cassert>
#include <cmath>

using namespace bluesky::planning;

int main() {
    PlanningGraph graph;
    graph.start_node = "A";
    graph.goal_node = "C";
    graph.nodes = {
        {"A", 0.0, 0.0, {59.0, 30.0}, 20.0},
        {"B", 1.0, 1.0, {59.001, 30.002}, 80.0},
        {"C", 2.0, 2.0, {59.003, 30.004}, 100.0}
    };
    graph.edges = {{"A", "B", 1.0}, {"B", "C", 1.0}};

    SelectedRouteSet selected;
    selected.mission_id = "MISSION-1";
    selected.mission_version = "v2";
    selected.candidate_id = "CANDIDATE-7";
    selected.route_elements = {"A", "B", "C"};
    selected.calculation_input_version = "ENV-9";

    const auto built = CanonicalRouteBuilder::build(selected, graph, "ROUTE-7", "3");
    assert(built.valid);
    assert(built.error.empty());
    assert(built.route.lineage.mission_id == "MISSION-1");
    assert(built.route.lineage.calculation_input_version == "ENV-9");
    assert(built.route.waypoints.size() == 3);
    assert(built.route.waypoints[1].position.latitude_deg == 59.001);
    assert(built.route.waypoints[1].altitude_m == 80.0);
    assert(built.route.waypoints.front().mandatory);
    assert(built.route.waypoints.back().mandatory);
    assert(built.route.segments.size() == 2);
    assert(built.route.segments.front().distance_m > 0.0);

    auto missing = selected;
    missing.route_elements = {"A", "NO-SUCH-NODE"};
    const auto rejectedMissing = CanonicalRouteBuilder::build(missing, graph, "R", "1");
    assert(!rejectedMissing.valid);
    assert(rejectedMissing.error == "SELECTED_ROUTE_ELEMENT_NOT_FOUND:NO-SUCH-NODE");

    auto duplicate = selected;
    duplicate.route_elements = {"A", "A"};
    const auto rejectedDuplicate = CanonicalRouteBuilder::build(duplicate, graph, "R", "1");
    assert(!rejectedDuplicate.valid);
    assert(rejectedDuplicate.error == "EMPTY_OR_DUPLICATE_ROUTE_ELEMENT");

    auto shortRoute = selected;
    shortRoute.route_elements = {"A"};
    const auto rejectedShort = CanonicalRouteBuilder::build(shortRoute, graph, "R", "1");
    assert(!rejectedShort.valid);
    assert(rejectedShort.error == "SELECTED_ROUTE_REQUIRES_AT_LEAST_TWO_ELEMENTS");

    graph.nodes[1].position.latitude_deg = 95.0;
    const auto rejectedCoordinate = CanonicalRouteBuilder::build(selected, graph, "R", "1");
    assert(!rejectedCoordinate.valid);
    assert(rejectedCoordinate.error == "INVALID_GEOGRAPHIC_ROUTE_NODE:B");
    return 0;
}
