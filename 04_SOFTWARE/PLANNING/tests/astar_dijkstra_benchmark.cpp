#include "../solver_contract/solver_contract.hpp"
#include "../solver_contract/solver_comparison.hpp"
#include "../solvers/astar_solver.hpp"
#include "../solvers/dijkstra_solver.hpp"

#include <cassert>
#include <chrono>
#include <cmath>
#include <iostream>
#include <utility>
#include <vector>

using namespace bluesky::planning;

class TestContext final : public SolverContext {
public:
    TestContext(MissionProblem problem, ComputeBudget budget)
        : problem_(std::move(problem)), budget_(budget) {}

    const MissionProblem& problem() const override { return problem_; }
    const ComputeBudget& budget() const override { return budget_; }
    bool cancelled() const override { return cancelled_; }
    void publish(CandidateSolution candidate) override {
        candidates_.push_back(std::move(candidate));
    }
    const std::vector<CandidateSolution>& candidates() const override {
        return candidates_;
    }

    MissionProblem problem_;
    ComputeBudget budget_;
    bool cancelled_{false};
    std::vector<CandidateSolution> candidates_;
};

static PlanningGraph make_graph() {
    PlanningGraph graph;
    graph.nodes = {
        {"S", 0.0, 0.0}, {"A", 1.0, 0.0}, {"B", 0.0, 1.0},
        {"C", 1.0, 1.0}, {"D", 2.0, 1.0}, {"G", 2.0, 2.0}
    };
    graph.edges = {
        {"S", "A", 2.0}, {"S", "B", 5.0},
        {"A", "B", 1.0}, {"A", "C", 2.0},
        {"B", "C", 1.0}, {"B", "D", 3.0},
        {"C", "D", 1.0}, {"C", "G", 6.0},
        {"D", "G", 1.0}
    };
    graph.start_node = "S";
    graph.goal_node = "G";
    return graph;
}

int main() {
    const auto graph = make_graph();
    MissionProblem problem{
        "benchmark-astar-dijkstra", "1", "point_to_point", {"UAV-01"},
        {}, {"route_efficiency"}, "test-environment", &graph
    };
    const ComputeBudget budget{1000, 128, 1};

    // Zero heuristic is the reference mode: A* becomes Dijkstra-equivalent,
    // making this test a correctness check independent of heuristic quality.
    AStarSolver astar([](const PlanningNode&, const PlanningNode&) {
        return 0.0;
    });
    DijkstraSolver dijkstra;

    TestContext a_ctx(problem, budget);
    TestContext d_ctx(problem, budget);

    const auto a_start = std::chrono::steady_clock::now();
    const auto a_state = astar.run(a_ctx);
    const auto a_us = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::steady_clock::now() - a_start).count();

    const auto d_start = std::chrono::steady_clock::now();
    const auto d_state = dijkstra.run(d_ctx);
    const auto d_us = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::steady_clock::now() - d_start).count();

    assert(a_state == RunState::Completed);
    assert(d_state == RunState::Completed);
    assert(a_ctx.candidates_.size() == 1);
    assert(d_ctx.candidates_.size() == 1);

    const auto& a = a_ctx.candidates_.front();
    const auto& d = d_ctx.candidates_.front();
    assert(a.feasibility == Feasibility::Feasible);
    assert(d.feasibility == Feasibility::Feasible);
    assert(std::fabs(a.estimated_time_s - d.estimated_time_s) < 1e-9);
    assert(std::fabs(d.estimated_time_s - 6.0) < 1e-9);
    assert(a.route_elements == d.route_elements);

    const auto* best = SolverCandidateEvaluator::best(a, d, 0.0, 100000.0);
    assert(best != nullptr);

    std::cout << "A* route cost: " << a.estimated_time_s
              << ", time(us): " << a_us << '\n';
    std::cout << "Dijkstra route cost: " << d.estimated_time_s
              << ", time(us): " << d_us << '\n';
    std::cout << "Benchmark: PASS\n";
    return 0;
}
