#include "constrained_planning_graph.hpp"

#include <cassert>

using namespace bluesky::planning;

int main()
{
    ConstrainedEnvironmentSnapshot environment;
    environment.snapshot_id = "ENV-1";
    environment.snapshot_version = "v1";
    environment.calculation_input_version = "INPUT-1";
    environment.complete = true;

    SpatialRestriction restriction;
    restriction.restriction_id = "R-1";
    restriction.source_id = "TEST";
    restriction.snapshot_version = "v1";
    restriction.geometry_type = RestrictionGeometryType::Circle;
    restriction.center = {59.0, 30.0};
    restriction.radius_m = 100.0;
    restriction.minimum_altitude_m = 0.0;
    restriction.maximum_altitude_m = 200.0;
    restriction.active = true;
    restriction.altitude_reference = AltitudeReference::AGL;
    environment.restrictions.push_back(restriction);

    PlanningGraph graph;
    graph.nodes.push_back({"N1", 0.0, 0.0, {59.0, 30.0}, 100.0, AltitudeReference::AGL});
    graph.start_node = "MISSING-START";
    graph.goal_node = "N1";

    // Missing endpoints must be rejected safely, not crash while computing
    // environment dependency identity.
    const auto result = ConstrainedPlanningGraphBuilder::build(graph, environment);
    assert(result.graph.start_node.empty());
    assert(result.graph.goal_node.empty());
    assert(result.dependency_identity.empty());

    graph.start_node = "N1";
    graph.goal_node = "MISSING-GOAL";
    const auto missingGoal = ConstrainedPlanningGraphBuilder::build(graph, environment);
    assert(missingGoal.graph.start_node.empty());
    assert(missingGoal.graph.goal_node.empty());
    assert(missingGoal.dependency_identity.empty());

    // The constrained graph must retain a lateral detour while excluding
    // a direct edge through a prohibited footprint. Search solvers can then
    // find the detour without weakening the restriction.
    ConstrainedEnvironmentSnapshot detourEnvironment;
    detourEnvironment.snapshot_id = "ENV-DETOUR";
    detourEnvironment.snapshot_version = "v-detour";
    detourEnvironment.calculation_input_version = "INPUT-DETOUR";
    SpatialRestriction detourZone;
    detourZone.restriction_id = "NOTAM-DETOUR";
    detourZone.source_id = "TEST";
    detourZone.snapshot_version = "NOTAM-v1";
    detourZone.geometry_type = RestrictionGeometryType::Circle;
    detourZone.center = {59.0, 30.0};
    detourZone.radius_m = 250.0;
    detourZone.minimum_altitude_m = 0.0;
    detourZone.maximum_altitude_m = 200.0;
    detourZone.active = true;
    detourZone.altitude_reference = AltitudeReference::AGL;
    detourEnvironment.restrictions.push_back(detourZone);

    PlanningGraph detourGraph;
    detourGraph.nodes = {
        {"S", 0, 0, {59.0, 29.995}, 100.0, AltitudeReference::AGL},
        {"G", 1, 0, {59.0, 30.005}, 100.0, AltitudeReference::AGL},
        {"D1", 0.5, 1, {59.004, 30.0}, 100.0, AltitudeReference::AGL},
        {"D2", 0.5, 2, {59.004, 30.004}, 100.0, AltitudeReference::AGL}
    };
    detourGraph.start_node = "S";
    detourGraph.goal_node = "G";
    detourGraph.edges = {
        {"S", "G", 1.0}, {"S", "D1", 1.0}, {"D1", "D2", 1.0},
        {"D2", "G", 1.0}
    };
    const auto detourResult =
        ConstrainedPlanningGraphBuilder::build(detourGraph, detourEnvironment);
    assert(detourResult.rejected_edges == 1);
    assert(detourResult.graph.edges.size() == 3);
    bool directEdgeRetained = false;
    for (const auto& edge : detourResult.graph.edges)
        if (edge.from == "S" && edge.to == "G") directEdgeRetained = true;
    assert(!directEdgeRetained);

    // Graph edges must carry an explicit, compatible altitude datum before
    // numerical comparison with restriction altitude limits is permitted.
    graph.nodes.push_back({"N2", 100.0, 100.0, {59.02, 30.02}, 100.0});
    graph.start_node = "N1";
    graph.goal_node = "N2";
    graph.edges = {{"N1", "N2", 1.0}};
    auto unrestricted = environment;
    unrestricted.restrictions.clear();

    const auto unknownDatumGraph =
        ConstrainedPlanningGraphBuilder::build(graph, unrestricted);
    assert(unknownDatumGraph.graph.edges.empty());
    assert(unknownDatumGraph.rejected_edges == 1);

    graph.nodes.back().altitude_reference = AltitudeReference::AGL;
    const auto explicitAglGraph =
        ConstrainedPlanningGraphBuilder::build(graph, unrestricted);
    assert(explicitAglGraph.graph.edges.size() == 1);
    assert(explicitAglGraph.rejected_edges == 0);
    // Environment lineage must remain available even when a complete snapshot
    // legitimately contains no active restrictions.
    assert(!explicitAglGraph.dependency_identity.empty());

    return 0;
}
