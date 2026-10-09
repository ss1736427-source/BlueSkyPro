#pragma once
#include "constrained_open_space.hpp"
#include "coverage_decomposition.hpp"
#include <string>
#include <vector>
namespace bluesky::planning {
struct CoverageTransitionEdge {
    std::string from_track_id;
    std::string to_track_id;
    double distance_m{0.0};
    double cost_m{0.0};
};
struct CoverageTransitionGraphResult {
    bool valid{false};
    std::string failure_code;
    std::string dependency_identity;
    std::vector<std::string> track_ids;
    std::vector<CoverageTransitionEdge> edges;
    std::size_t rejected_edges{0};
};
struct CoverageTransitionGraphInput {
    CoverageTrackResult tracks;
    ConstrainedEnvironmentSnapshot environment;
    std::string calculation_version;
};
class CoverageTransitionGraphBuilder final {
public:
    static CoverageTransitionGraphResult build(const CoverageTransitionGraphInput& input);
};
} // namespace bluesky::planning
