#include "coverage_transition_graph.hpp"
#include <cmath>
#include <algorithm>
namespace bluesky::planning {
namespace {
constexpr double kPi=3.14159265358979323846;
constexpr double kMetersPerDegree=111320.0;
double distance(const GeoPoint& a,const GeoPoint& b){
    const double lat=(a.latitude_deg+b.latitude_deg)*0.5*kPi/180.0;
    const double dx=(b.longitude_deg-a.longitude_deg)*kMetersPerDegree*std::cos(lat);
    const double dy=(b.latitude_deg-a.latitude_deg)*kMetersPerDegree;
    return std::sqrt(dx*dx+dy*dy);
}
}
CoverageTransitionGraphResult CoverageTransitionGraphBuilder::build(const CoverageTransitionGraphInput& input){
    CoverageTransitionGraphResult result;
    if(!input.tracks.valid){result.failure_code="INVALID_TRACK_INPUT";return result;}
    if(input.tracks.tracks.empty()){result.failure_code="NO_COVERAGE_TRACK";return result;}
    result.dependency_identity=input.tracks.dependency_identity+"|"+input.calculation_version;
    for(const auto& track:input.tracks.tracks){
        if(track.track_id.empty() ||
           !std::isfinite(track.start.latitude_deg) || !std::isfinite(track.start.longitude_deg) ||
           !std::isfinite(track.end.latitude_deg) || !std::isfinite(track.end.longitude_deg) ||
           !std::isfinite(track.altitude_m) || !(track.length_m>0.0) || !std::isfinite(track.length_m)){
            result.failure_code="INVALID_COVERAGE_TRACK";return result;
        }
        result.track_ids.push_back(track.track_id);
    }
    // Controlled first slice: explicit DISTANCE_ONLY cost; no hidden weights.
    for(const auto& from:input.tracks.tracks){
        for(const auto& to:input.tracks.tracks){
            if(from.track_id==to.track_id) continue;
            if(std::abs(from.altitude_m-to.altitude_m)>1e-9){++result.rejected_edges;continue;}
            const auto check=ConstrainedOpenSpace::evaluateSegment(
                input.environment,{from.end,to.start,from.altitude_m,from.altitude_m,from.altitude_m});
            if(!check.allowed){++result.rejected_edges;continue;}
            const double d=distance(from.end,to.start);
            if(!(std::isfinite(d)&&d>=0.0)){result.failure_code="INVALID_TRANSITION_DISTANCE";return result;}
            result.edges.push_back({from.track_id,to.track_id,d,d});
        }
    }
    result.valid=true;
    return result;
}
} // namespace bluesky::planning


namespace bluesky::planning {
namespace {
CoverageRouteCandidate buildGreedyCandidate(
    const CoverageTransitionGraphResult& graph,
    std::size_t start_index,
    bool& complete) {
    CoverageRouteCandidate candidate;
    complete = false;
    if (start_index >= graph.track_ids.size()) return candidate;

    std::vector<bool> used(graph.track_ids.size(), false);
    std::size_t current_index = start_index;
    used[current_index] = true;
    candidate.track_ids.push_back(graph.track_ids[current_index]);

    while (candidate.track_ids.size() < graph.track_ids.size()) {
        const std::string& current = graph.track_ids[current_index];
        const CoverageTransitionEdge* best = nullptr;
        std::size_t best_index = graph.track_ids.size();

        for (const auto& edge : graph.edges) {
            if (edge.from_track_id != current) continue;
            std::size_t target_index = graph.track_ids.size();
            for (std::size_t i = 0; i < graph.track_ids.size(); ++i) {
                if (graph.track_ids[i] == edge.to_track_id) {
                    target_index = i;
                    break;
                }
            }
            if (target_index >= used.size() || used[target_index]) continue;
            if (!best || edge.cost_m < best->cost_m ||
                (edge.cost_m == best->cost_m && edge.to_track_id < best->to_track_id)) {
                best = &edge;
                best_index = target_index;
            }
        }

        if (!best) return candidate;

        candidate.transitions.push_back(*best);
        candidate.transition_cost_m += best->cost_m;
        candidate.track_ids.push_back(best->to_track_id);
        used[best_index] = true;
        current_index = best_index;
    }

    complete = true;
    return candidate;
}
bool candidateLess(const CoverageRouteCandidate& a, const CoverageRouteCandidate& b) {
    if (a.transition_cost_m != b.transition_cost_m)
        return a.transition_cost_m < b.transition_cost_m;
    return a.track_ids < b.track_ids;
}
}

CoverageRouteCandidateResult CoverageRouteCandidateBuilder::generate(
    const CoverageRouteCandidateInput& input) {
    CoverageRouteCandidateResult result;
    if (!input.graph.valid) { result.failure_code = "INVALID_TRANSITION_GRAPH"; return result; }
    if (input.graph.track_ids.empty()) { result.failure_code = "NO_COVERAGE_TRACK"; return result; }
    if (input.max_candidates == 0) { result.failure_code = "INVALID_CANDIDATE_LIMIT"; return result; }

    result.dependency_identity =
        input.graph.dependency_identity + "|ROUTES|" + input.calculation_version;

    for (std::size_t start = 0; start < input.graph.track_ids.size(); ++start) {
        bool complete = false;
        auto candidate = buildGreedyCandidate(input.graph, start, complete);
        if (complete) result.candidates.push_back(std::move(candidate));
    }

    std::sort(result.candidates.begin(), result.candidates.end(), candidateLess);
    if (result.candidates.size() > input.max_candidates)
        result.candidates.resize(input.max_candidates);

    if (result.candidates.empty()) {
        result.failure_code = "NO_COMPLETE_ROUTE_CANDIDATE";
        return result;
    }

    result.valid = true;
    return result;
}
} // namespace bluesky::planning
