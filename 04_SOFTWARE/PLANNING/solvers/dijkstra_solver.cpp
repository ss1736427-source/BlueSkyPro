#include "dijkstra_solver.hpp"

#include <algorithm>
#include <chrono>
#include <functional>
#include <limits>
#include <queue>
#include <unordered_map>

namespace bluesky::planning {

SolverMetadata DijkstraSolver::metadata() const {
    return SolverMetadata{
        "dijkstra",
        "1.0.1",
        {"point_to_point", "graph_route", "constrained_graph_route"},
        false,
        true
    };
}

bool DijkstraSolver::eligible(const MissionProblem& problem) const {
    return (problem.problem_class == "point_to_point" ||
            problem.problem_class == "graph_route" ||
            problem.problem_class == "constrained_graph_route") &&
           problem.planning_graph != nullptr;
}

RunState DijkstraSolver::run(SolverContext& context) {
    cancelled_.store(false);

    if (!eligible(context.problem())) return RunState::Failed;
    const auto& graph_data = *context.problem().planning_graph;
    const auto* start = graph_data.find_node(graph_data.start_node);
    const auto* goal = graph_data.find_node(graph_data.goal_node);
    if (!start || !goal || context.budget().max_candidates == 0)
        return RunState::Failed;

    const auto started = std::chrono::steady_clock::now();
    const auto budget_ms = context.budget().wall_time_ms;
    const double inf = std::numeric_limits<double>::infinity();

    std::unordered_map<std::string, std::vector<PlanningGraphEdge>> adjacency;
    for (const auto& edge : graph_data.edges) {
        if (edge.cost >= 0.0 && graph_data.find_node(edge.from) && graph_data.find_node(edge.to)) {
            adjacency[edge.from].push_back(edge);
        }
    }

    std::unordered_map<std::string, double> distance;
    std::unordered_map<std::string, std::string> parent;
    for (const auto& node : graph_data.nodes) distance[node.id] = inf;
    distance[start->id] = 0.0;

    struct QueueItem {
        std::string id;
        double distance;
        bool operator>(const QueueItem& other) const {
            return distance > other.distance;
        }
    };

    std::priority_queue<QueueItem, std::vector<QueueItem>, std::greater<>> open;
    open.push({start->id, 0.0});

    while (!open.empty()) {
        if (cancelled_.load() || context.cancelled())
            return RunState::Cancelled;

        if (budget_ms > 0) {
            const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - started).count();
            if (static_cast<std::uint64_t>(elapsed) >= budget_ms)
                return RunState::Timeout;
        }

        const auto current = open.top();
        open.pop();
        if (current.distance != distance[current.id]) continue;

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
            candidate.candidate_id = context.problem().mission_id + ":dijkstra:1";
            candidate.solver_id = metadata().solver_id;
            candidate.solver_version = metadata().version;
            candidate.route_elements = std::move(route);
            candidate.estimated_time_s = current.distance;
            candidate.objective_score = current.distance;
            candidate.feasibility = Feasibility::Feasible;
            context.publish(std::move(candidate));
            return RunState::Completed;
        }

        for (const auto& edge : adjacency[current.id]) {
            const double next_distance = current.distance + edge.cost;
            const auto it = distance.find(edge.to);
            if (it != distance.end() && next_distance < it->second) {
                it->second = next_distance;
                parent[edge.to] = current.id;
                open.push({edge.to, next_distance});
            }
        }
    }

    return RunState::Failed;
}

void DijkstraSolver::cancel() {
    cancelled_.store(true);
}

} // namespace bluesky::planning
