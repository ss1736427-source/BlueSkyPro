#pragma once

#include "../model/planning_graph.hpp"
#include <cmath>

namespace bluesky::planning {

// Admissible for graphs whose edge cost is not less than Euclidean distance.
inline double euclidean_heuristic(const PlanningNode& from,
                                  const PlanningNode& goal) {
    const double dx = from.x - goal.x;
    const double dy = from.y - goal.y;
    return std::sqrt(dx * dx + dy * dy);
}

inline double zero_heuristic(const PlanningNode&, const PlanningNode&) {
    return 0.0;
}

} // namespace bluesky::planning
