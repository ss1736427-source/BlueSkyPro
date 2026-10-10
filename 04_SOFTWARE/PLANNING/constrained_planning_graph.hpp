#pragma once
#include "constrained_open_space.hpp"
#include "model/planning_graph.hpp"
#include <string>

namespace bluesky::planning {

struct ConstrainedGraphBuildResult {
    PlanningGraph graph;
    std::string environment_snapshot_id;
    std::string environment_snapshot_version;
    std::string dependency_identity;
    std::size_t rejected_edges{0};
};

class ConstrainedPlanningGraphBuilder {
public:
    static ConstrainedGraphBuildResult build(
        const PlanningGraph& source,
        const ConstrainedEnvironmentSnapshot& environment);
};

} // namespace bluesky::planning
