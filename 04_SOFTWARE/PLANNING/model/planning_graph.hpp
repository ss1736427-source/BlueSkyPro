#pragma once

#include <algorithm>
#include "route_model.hpp"
#include <string>
#include <vector>

namespace bluesky::planning {

struct PlanningNode {
    std::string id;
    double x{0.0};
    double y{0.0};
    GeoPoint position;
    double altitude_m{0.0};
};

struct PlanningGraphEdge {
    std::string from;
    std::string to;
    double cost{0.0};
};

struct PlanningGraph {
    std::vector<PlanningNode> nodes;
    std::vector<PlanningGraphEdge> edges;
    std::string start_node;
    std::string goal_node;

    const PlanningNode* find_node(const std::string& id) const {
        const auto it = std::find_if(nodes.begin(), nodes.end(),
            [&](const PlanningNode& node) { return node.id == id; });
        return it == nodes.end() ? nullptr : &*it;
    }
};

} // namespace bluesky::planning
