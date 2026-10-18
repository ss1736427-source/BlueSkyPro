#include "constrained_planning_graph.hpp"

namespace bluesky::planning {
ConstrainedGraphBuildResult ConstrainedPlanningGraphBuilder::build(
    const PlanningGraph& source,
    const ConstrainedEnvironmentSnapshot& environment) {
    ConstrainedGraphBuildResult result;
    result.graph.start_node = source.start_node;
    result.graph.goal_node = source.goal_node;
    result.environment_snapshot_id = environment.snapshot_id;
    result.environment_snapshot_version = environment.snapshot_version;

    const auto open = [&](const PlanningGraphEdge& edge) {
        const auto* from = source.find_node(edge.from);
        const auto* to = source.find_node(edge.to);
        if (!from || !to) return false;
        const auto altitude_reference = from->altitude_reference == to->altitude_reference
            ? from->altitude_reference : AltitudeReference::Unknown;
        const SpatialEdge spatial{from->position, to->position,
                                  (from->altitude_m + to->altitude_m) * 0.5,
                                  std::min(from->altitude_m, to->altitude_m),
                                  std::max(from->altitude_m, to->altitude_m),
                                  altitude_reference};
        return ConstrainedOpenSpace::evaluateSegment(environment, spatial).allowed;
    };

    for (const auto& node : source.nodes)
        result.graph.nodes.push_back(node);

    for (const auto& edge : source.edges) {
        if (open(edge)) result.graph.edges.push_back(edge);
        else ++result.rejected_edges;
    }

    if (source.find_node(source.start_node) == nullptr ||
        source.find_node(source.goal_node) == nullptr) {
        result.graph.start_node.clear();
        result.graph.goal_node.clear();
    }

    // Invalid or unresolved endpoints are a normal rejected-input path. Do not
    // dereference absent nodes while trying to compute environment lineage.
    const auto* startNode = result.graph.find_node(result.graph.start_node);
    const auto* goalNode = result.graph.find_node(result.graph.goal_node);
    if (environment.complete && !environment.snapshot_id.empty()
        && !environment.snapshot_version.empty()
        && !environment.calculation_input_version.empty()
        && startNode && goalNode) {
        const auto probe = ConstrainedOpenSpace::evaluateSegment(
            environment,
            {startNode->position, goalNode->position, 0.0});
        result.dependency_identity = probe.dependency_identity;
    }
    return result;
}
} // namespace bluesky::planning
