#include "coverage_transition_graph.hpp"
#include "candidate_comparison.hpp"
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

namespace {
Route buildRoute(
    const CoverageRouteCandidate& candidate,
    const CoverageTrackResult& tracks,
    std::size_t candidateIndex) {
    Route route;
    route.lineage.route_id = "MT01-ROUTE-" + std::to_string(candidateIndex);
    route.lineage.route_version = "1";
    route.lineage.generator_id = "MT01-COVERAGE-ROUTE-CANDIDATE";
    route.lineage.generator_version = "1";
    route.lineage.calculation_input_version = tracks.dependency_identity;

    const CoverageTrack* first = nullptr;
    for (const auto& track : tracks.tracks) {
        if (track.track_id == candidate.track_ids.front()) {
            first = &track;
            break;
        }
    }
    if (!first) return route;

    route.waypoints.push_back({"MT01-WP-0", first->start, first->altitude_m, false});
    route.waypoints.push_back({"MT01-WP-1", first->end, first->altitude_m, false});

    std::size_t waypointIndex = 2;
    for (std::size_t i = 1; i < candidate.track_ids.size(); ++i) {
        const CoverageTrack* track = nullptr;
        for (const auto& item : tracks.tracks) {
            if (item.track_id == candidate.track_ids[i]) {
                track = &item;
                break;
            }
        }
        if (!track) return Route{};

        route.waypoints.push_back({
            "MT01-WP-" + std::to_string(waypointIndex++),
            track->start, track->altitude_m, false});
        route.waypoints.push_back({
            "MT01-WP-" + std::to_string(waypointIndex++),
            track->end, track->altitude_m, false});
    }

    std::size_t segmentIndex = 0;
    for (std::size_t i = 0; i < candidate.track_ids.size(); ++i) {
        const std::size_t base = i * 2;
        const auto& from = route.waypoints[base];
        const auto& to = route.waypoints[base + 1];
        route.segments.push_back({
            "MT01-TRACK-SEG-" + std::to_string(segmentIndex++),
            from.waypoint_id, to.waypoint_id, 0.0, 0.0});

        if (i + 1 < candidate.track_ids.size()) {
            const auto& transition = candidate.transitions[i];
            const auto& transitionFrom = route.waypoints[base + 1];
            const auto& transitionTo = route.waypoints[base + 2];
            route.segments.push_back({
                "MT01-TRANSITION-SEG-" + transition.from_track_id + "-" +
                    transition.to_track_id,
                transitionFrom.waypoint_id, transitionTo.waypoint_id,
                transition.distance_m, 0.0});
        }
    }
    return route;
}
}

CoverageRoutePerformanceResult CoverageRoutePerformanceEvaluator::evaluate(
    const CoverageRoutePerformanceInput& input) {
    CoverageRoutePerformanceResult result;
    if (!input.tracks.valid) {
        result.failure_code = "INVALID_TRACK_INPUT";
        return result;
    }
    if (!input.candidates.valid || input.candidates.candidates.empty()) {
        result.failure_code = "INVALID_ROUTE_CANDIDATES";
        return result;
    }

    result.dependency_identity =
        input.candidates.dependency_identity + "|PERFORMANCE|" +
        input.performance.uav_id + "|" + input.performance.wind_snapshot_id +
        "|" + input.calculation_version;

    for (std::size_t i = 0; i < input.candidates.candidates.size(); ++i) {
        const auto& candidate = input.candidates.candidates[i];
        if (candidate.track_ids.empty() ||
            candidate.transitions.size() + 1 != candidate.track_ids.size()) {
            result.failure_code = "INVALID_ROUTE_CANDIDATE";
            return result;
        }

        Route route = buildRoute(candidate, input.tracks, i);
        if (route.waypoints.empty() ||
            route.segments.size() + 1 != route.waypoints.size()) {
            result.failure_code = "ROUTE_BUILD_FAILED";
            return result;
        }

        result.evaluations.push_back(
            WindPerformanceTrajectory::calculate(
                route, input.performance, input.calculation_version));
    }

    result.valid = true;
    return result;
}

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

namespace {
CandidateSolution toCandidate(
    const TrajectoryResult& trajectory,
    std::size_t index) {
    CandidateSolution candidate;
    candidate.candidate_id = trajectory.route_id.empty()
        ? "MT01-ROUTE-CANDIDATE-" + std::to_string(index)
        : trajectory.route_id;
    candidate.solver_id = "MT01-WIND-PERFORMANCE";
    candidate.solver_version = trajectory.calculation_version;
    candidate.estimated_time_s = trajectory.total_time_s;
    candidate.estimated_energy_wh = trajectory.total_energy_wh;
    candidate.estimated_reserve_wh = trajectory.remaining_energy_wh;
    candidate.feasibility =
        trajectory.status == TrajectoryStatus::Feasible
            ? Feasibility::Feasible : Feasibility::Infeasible;
    for (const auto& finding : trajectory.findings)
        candidate.constraint_violations.push_back(
            std::to_string(static_cast<int>(finding.code)) + ":" + finding.segment_id);
    return candidate;
}
}

CoverageRouteSelectionResult CoverageRouteSelector::select(
    const CoverageRoutePerformanceResult& performance,
    const std::vector<std::string>& objective_priorities,
    const std::string& calculation_input_version,
    const std::string& calculation_version) {
    CoverageRouteSelectionResult result;
    if (!performance.valid || performance.evaluations.empty()) {
        result.failure_code = "INVALID_PERFORMANCE_RESULTS";
        return result;
    }

    CandidateComparisonInput comparisonInput;
    comparisonInput.calculation_input_version = calculation_input_version;
    comparisonInput.objective_priorities = objective_priorities;
    for (std::size_t i = 0; i < performance.evaluations.size(); ++i)
        comparisonInput.candidates.push_back(
            toCandidate(performance.evaluations[i], i));

    const auto comparison =
        CandidateComparator::compare(comparisonInput, calculation_version);
    result.dependency_identity =
        performance.dependency_identity + "|SELECTION|" +
        calculation_input_version + "|" + calculation_version;
    result.rejected_route_ids = comparison.rejected_candidate_ids;

    if (!comparison.feasible) {
        result.failure_code = "NO_FEASIBLE_ROUTE_CANDIDATE";
        return result;
    }

    result.selected_route_id = comparison.selected_candidate_id;
    result.valid = true;
    return result;
}


