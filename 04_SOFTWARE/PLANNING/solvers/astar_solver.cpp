#include "astar_solver.hpp"

#include <algorithm>
#include <chrono>
#include <functional>
#include <limits>
#include <queue>
#include <unordered_map>
#include <unordered_set>

namespace bluesky::planning {

AStarSolver::AStarSolver(Heuristic heuristic)
    : heuristic_(std::move(heuristic)) {}

SolverMetadata AStarSolver::metadata() const {
    return {"astar", "1.1.0", {"point_to_point", "graph_route", "constrained_graph_route"}, false, true};
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

    std::unordered_map<std::string, std::vector<PlanningGraphEdge>> adjacency;
    for (const auto& edge : graph.edges) {
        if (edge.cost >= 0.0) adjacency[edge.from].push_back(edge);
    }

    std::unordered_map<std::string, double> g;
    std::unordered_map<std::string, std::string> parent;
    for (const auto& node : graph.nodes) g[node.id] = inf;
    g[start->id] = 0.0;

    struct QueueItem {
        std::string id;
        double f;
        bool operator>(const QueueItem& other) const { return f > other.f; }
    };

    std::priority_queue<QueueItem, std::vector<QueueItem>, std::greater<>> open;
    open.push({start->id, heuristic_(*start, *goal)});

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
            if (closed.contains(edge.to)) continue;
            const double tentative_g = g[current.id] + edge.cost;
            if (tentative_g < g[edge.to]) {
                g[edge.to] = tentative_g;
                parent[edge.to] = current.id;
                const auto* next_node = graph.find_node(edge.to);
                if (!next_node) return RunState::Failed;
                open.push({edge.to, tentative_g + heuristic_(*next_node, *goal)});
            }
        }
    }

    return RunState::Failed;
}

void AStarSolver::cancel() { cancelled_.store(true); }

} // namespace bluesky::planning
