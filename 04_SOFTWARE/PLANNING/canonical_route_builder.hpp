#pragma once

#include "model/planning_graph.hpp"
#include "selected_route_set.hpp"
#include "model/route_model.hpp"

#include <string>

namespace bluesky::planning {

struct CanonicalRouteBuildResult {
    bool valid{false};
    Route route;
    std::string error;
};

class CanonicalRouteBuilder {
public:
    // Resolves selected route-element IDs against the authoritative planning graph.
    // The graph nodes' geographic positions are preserved; x/y are never interpreted as WGS84.
    static CanonicalRouteBuildResult build(
        const SelectedRouteSet& selected,
        const PlanningGraph& graph,
        const std::string& route_id,
        const std::string& route_version,
        const std::string& generator_version = "1.0.0");
};

} // namespace bluesky::planning
