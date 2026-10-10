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
        const SpatialEdge spatial{from->position, to->position,
                                  (from->altitude_m + to->altitude_m) * 0.5,
                                  std::min(from->altitude_m, to->altitude_m),
                                  std::max(from->altitude_m, to->altitude_m)};
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

    if (environment.complete && !environment.restrictions.empty()) {
        const auto probe = ConstrainedOpenSpace::evaluateSegment(
            environment,
            {result.graph.find_node(source.start_node)->position,
             result.graph.find_node(source.goal_node)->position,
             0.0});
        result.dependency_identity = probe.dependency_identity;
    }
    return result;
}
} // namespace bluesky::planning
