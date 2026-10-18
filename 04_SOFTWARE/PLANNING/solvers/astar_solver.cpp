#include "astar_solver.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <functional>
#include <limits>
#include <queue>
#include <unordered_map>
#include <unordered_set>

namespace bluesky::planning {

AStarSolver::AStarSolver(Heuristic heuristic)
    : heuristic_(std::move(heuristic)) {}

SolverMetadata AStarSolver::metadata() const {
    return {"astar", "1.1.1", {"point_to_point", "graph_route", "constrained_graph_route"}, false, true};
}

bool AStarSolver::eligible(const MissionProblem& problem) const {
    return (problem.problem_class == "point_to_point" ||
            problem.problem_class == "graph_route" ||
            problem.problem_class == "constrained_graph_route") &&
           problem.planning_graph != nullptr && static_cast<bool>(heuristic_);
}

RunState AStarSolver::run(SolverContext& context) {
    cancelled_.store(false);
    if (!eligible(context.problem())) return RunState::Failed;

    const auto& graph = *context.problem().planning_graph;
    const auto* start = graph.find_node(graph.start_node);
    const auto* goal = graph.find_node(graph.goal_node);
    if (!start || !goal || context.budget().max_candidates == 0) return RunState::Failed;

    const auto started = std::chrono::steady_clock::now();
    const auto budget_ms = context.budget().wall_time_ms;
    const double inf = std::numeric_limits<double>::infinity();

    // A* is defined here for finite, non-negative edge costs. Silently skipping
    // malformed edges can produce a plausible but incomplete route, so reject
    // the graph rather than optimizing over a silently altered graph.
    std::unordered_map<std::string, std::vector<PlanningGraphEdge>> adjacency;
    for (const auto& edge : graph.edges) {
        if (!std::isfinite(edge.cost) || edge.cost < 0.0) return RunState::Failed;
        adjacency[edge.from].push_back(edge);
    }

    const double start_h = heuristic_(*start, *goal);
    if (!std::isfinite(start_h)) return RunState::Failed;

    std::unordered_map<std::string, double> g;
    std::unordered_map<std::string, std::string> parent;
    for (const auto& node : graph.nodes) g[node.id] = inf;
    g[start->id] = 0.0;

    struct QueueItem {
        std::string id;
        double f;
        double g;
        bool operator>(const QueueItem& other) const { return f > other.f; }
    };

    std::priority_queue<QueueItem, std::vector<QueueItem>, std::greater<>> open;
    open.push({start->id, start_h, 0.0});

    std::unordered_set<std::string> closed;
    while (!open.empty()) {
        if (cancelled_.load() || context.cancelled()) return RunState::Cancelled;
        if (budget_ms > 0) {
            const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - started).count();
            if (static_cast<std::uint64_t>(elapsed) >= budget_ms) return RunState::Timeout;
        }

        const auto current = open.top();
        open.pop();
        const auto best_g = g.find(current.id);
        if (best_g == g.end() || current.g != best_g->second) continue;
        if (!std::isfinite(current.f) || !std::isfinite(current.g)) return RunState::Failed;
        if (closed.contains(current.id)) continue;
        closed.insert(current.id);

        if (current.id == goal->id) {
            std::vector<std::string> route;
            for (std::string at = current.id; !at.empty();) {
                route.push_back(at);
                const auto it = parent.find(at);
                if (it == parent.end()) break;
                at = it->second;
            }
            std::reverse(route.begin(), route.end());

            CandidateSolution candidate;
            candidate.candidate_id = context.problem().mission_id + ":astar:1";
            candidate.solver_id = metadata().solver_id;
            candidate.solver_version = metadata().version;
            candidate.route_elements = std::move(route);
            candidate.estimated_time_s = g[current.id];
            candidate.objective_score = g[current.id];
            candidate.feasibility = Feasibility::Feasible;
            context.publish(std::move(candidate));
            return RunState::Completed;
        }

        const auto* current_node = graph.find_node(current.id);
        if (!current_node) return RunState::Failed;

        for (const auto& edge : adjacency[current.id]) {
            const auto destination = g.find(edge.to);
            if (destination == g.end()) return RunState::Failed;

            const double tentative_g = g[current.id] + edge.cost;
            if (!std::isfinite(tentative_g)) return RunState::Failed;
            if (tentative_g < destination->second) {
                const auto* next_node = graph.find_node(edge.to);
                if (!next_node) return RunState::Failed;
                const double h = heuristic_(*next_node, *goal);
                if (!std::isfinite(h)) return RunState::Failed;
                const double f = tentative_g + h;
                if (!std::isfinite(f)) return RunState::Failed;

                g[edge.to] = tentative_g;
                parent[edge.to] = current.id;
                closed.erase(edge.to);
                open.push({edge.to, f, tentative_g});
            }
        }
    }

    return RunState::Failed;
}

void AStarSolver::cancel() { cancelled_.store(true); }

} // namespace bluesky::planning
