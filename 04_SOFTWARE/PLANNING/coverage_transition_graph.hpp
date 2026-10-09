#pragma once
#include "constrained_open_space.hpp"
#include "coverage_decomposition.hpp"
#include "wind_performance_trajectory.hpp"
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

struct CoverageRouteCandidate {
    std::vector<std::string> track_ids;
    std::vector<CoverageTransitionEdge> transitions;
    double transition_cost_m{0.0};
};

struct CoverageRouteCandidateResult {
    bool valid{false};
    std::string failure_code;
    std::string dependency_identity;
    std::vector<CoverageRouteCandidate> candidates;
};

struct CoverageRouteCandidateInput {
    CoverageTransitionGraphResult graph;
    std::size_t max_candidates{1};
    std::string calculation_version;
};

class CoverageRouteCandidateBuilder final {
public:
    static CoverageRouteCandidateResult generate(const CoverageRouteCandidateInput& input);
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


struct CoverageRoutePerformanceInput {
    CoverageTrackResult tracks;
    CoverageRouteCandidateResult candidates;
    TrajectoryPerformanceInput performance;
    std::string calculation_version;
};

struct CoverageRoutePerformanceResult {
    bool valid{false};
    std::string failure_code;
    std::string dependency_identity;
    std::vector<TrajectoryResult> evaluations;
};

class CoverageRoutePerformanceEvaluator final {
public:
    static CoverageRoutePerformanceResult evaluate(
        const CoverageRoutePerformanceInput& input);
};



struct CoverageRouteSelectionResult {
    bool valid{false};
    std::string failure_code;
    std::string dependency_identity;
    std::string selected_route_id;
    std::vector<std::string> rejected_route_ids;
};

class CoverageRouteSelector final {
public:
    static CoverageRouteSelectionResult select(
        const CoverageRoutePerformanceResult& performance,
        const std::vector<std::string>& objective_priorities,
        const std::string& calculation_input_version,
        const std::string& calculation_version);
};

} // namespace bluesky::planning
