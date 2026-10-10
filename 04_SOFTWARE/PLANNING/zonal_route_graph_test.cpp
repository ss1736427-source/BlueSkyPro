#include "zonal_route_graph.hpp"
#include <cassert>
#include <iostream>

using namespace bluesky::planning;

int main() {
    OperationalZone zone{
        "Z1", "v1", "AREA", 0,
        {{50.0, 10.0}, {51.0, 10.0}, {51.0, 11.0}, {50.0, 11.0}}
    };
    PlanningGraph graph;
    graph.nodes = {
        {"inside-a", 0, 0, {50.2, 10.2}, 100.0},
        {"inside-b", 0, 0, {50.8, 10.8}, 100.0},
        {"outside", 0, 0, {50.5, 11.5}, 100.0}
    };
    graph.start_node = "inside-a";
    graph.goal_node = "inside-b";
    graph.edges = {
        {"inside-a", "inside-b", 1.0},
        {"inside-a", "outside", 1.0}
    };

    const auto result = ZonalRouteGraphBuilder::build(graph, zone);
    assert(result.graph.nodes.size() == 2);
    assert(result.graph.edges.size() == 1);
    assert(result.graph.start_node == "inside-a");
    assert(result.graph.goal_node == "inside-b");
    assert(result.rejected_edges == 1);
    assert(!result.dependency_identity.empty());

    graph.start_node = "outside";
    const auto blocked = ZonalRouteGraphBuilder::build(graph, zone);
    assert(blocked.graph.start_node.empty());

    std::cout << "zonal_route_graph_test: PASS\n";
}
