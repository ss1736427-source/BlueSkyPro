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
    environment.restrictions.push_back(restriction);

    PlanningGraph graph;
    graph.nodes.push_back({"N1", 0.0, 0.0, {59.0, 30.0}, 100.0});
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

    return 0;
}
