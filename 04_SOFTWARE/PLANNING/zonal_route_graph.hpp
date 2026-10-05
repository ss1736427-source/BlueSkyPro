#pragma once

#include "model/planning_graph.hpp"
#include "multi_uav_zonal_planning.hpp"
#include <string>

namespace bluesky::planning {

struct ZonalRouteGraphResult {
    PlanningGraph graph;
    std::string zone_id;
    std::string zone_version;
    std::string dependency_identity;
    std::size_t rejected_nodes{0};
    std::size_t rejected_edges{0};
};

class ZonalRouteGraphBuilder {
public:
    static ZonalRouteGraphResult build(
        const PlanningGraph& source,
        const OperationalZone& zone);
};

} // namespace bluesky::planning
