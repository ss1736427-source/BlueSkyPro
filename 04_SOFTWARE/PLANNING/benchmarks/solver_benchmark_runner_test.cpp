#include "solver_benchmark_runner.hpp"
#include "../model/planning_graph.hpp"

#include <cassert>
#include <cmath>
#include <iostream>

using namespace bluesky::planning;
using namespace bluesky::planning::benchmark;

static PlanningGraph make_graph() {
    PlanningGraph g;
    g.nodes = {
        {"A", 0.0, 0.0},
        {"B", 1.0, 0.0},
        {"C", 2.0, 0.0},
        {"D", 1.0, 1.0},
        {"E", 2.0, 1.0}
    };
    g.edges = {
        {"A", "B", 1.0},
        {"B", "C", 1.0},
        {"A", "D", 1.5},
        {"D", "E", 0.5},
        {"E", "C", 1.0}
    };
    g.start_node = "A";
    g.goal_node = "C";
    return g;
}

int main() {
    const auto graph = make_graph();

    MissionProblem problem;
    problem.mission_id = "BENCH-001";
    problem.mission_version = "1.0";
    problem.problem_class = "point_to_point";
    problem.planning_graph = &graph;

    const auto report = run_astar_dijkstra_benchmark(problem, 3);
    assert(report.results.size() == 6);

    const BenchmarkResult* astar = nullptr;
    const BenchmarkResult* dijkstra = nullptr;
    for (const auto& result : report.results) {
        if (result.solver_id == "astar") astar = &result;
        if (result.solver_id == "dijkstra") dijkstra = &result;
    }

    assert(astar != nullptr);
    assert(dijkstra != nullptr);
    assert(astar->state == RunState::Completed);
    assert(dijkstra->state == RunState::Completed);
    assert(astar->feasibility == Feasibility::Feasible);
    assert(dijkstra->feasibility == Feasibility::Feasible);

    // Reference optimum for this graph is A -> B -> C = 2.0.
    assert(std::abs(dijkstra->route_cost - 2.0) < 1e-9);
    assert(std::abs(astar->route_cost - dijkstra->route_cost) < 1e-9);
    assert(astar->route_cost <= dijkstra->route_cost + 1e-9);

    std::cout << "solver benchmark: PASS\n";
    std::cout << "A*: " << astar->route_cost << " ms=" << astar->wall_time_ms << "\n";
    std::cout << "Dijkstra: " << dijkstra->route_cost << " ms=" << dijkstra->wall_time_ms << "\n";
    return 0;
}
