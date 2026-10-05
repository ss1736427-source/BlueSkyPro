#include "solver_benchmark_runner.hpp"
#include "../solvers/astar_solver.hpp"
#include "../solvers/dijkstra_solver.hpp"

#include <chrono>
#include <cmath>
#include <optional>
#include <stdexcept>

namespace bluesky::planning::benchmark {
namespace {

class BenchmarkContext final : public SolverContext {
public:
    BenchmarkContext(const MissionProblem& problem, ComputeBudget budget)
        : problem_(problem), budget_(budget) {}

    const MissionProblem& problem() const override { return problem_; }
    const ComputeBudget& budget() const override { return budget_; }
    bool cancelled() const override { return false; }
    void publish(CandidateSolution candidate) override {
        candidate_ = candidate;
        candidates_.push_back(std::move(candidate));
    }
    const std::vector<CandidateSolution>& candidates() const override { return candidates_; }

    const CandidateSolution* candidate() const {
        return candidate_.has_value() ? &*candidate_ : nullptr;
    }

private:
    const MissionProblem& problem_;
    ComputeBudget budget_;
    std::optional<CandidateSolution> candidate_;
    std::vector<CandidateSolution> candidates_;
};

BenchmarkResult run_one(Solver& solver, const MissionProblem& problem,
                        std::uint64_t repetition) {
    ComputeBudget budget;
    budget.wall_time_ms = 5000;
    budget.max_candidates = 1;

    BenchmarkContext context(problem, budget);
    const auto begin = std::chrono::steady_clock::now();
    const RunState state = solver.run(context);
    const auto end = std::chrono::steady_clock::now();

    BenchmarkResult result;
    result.solver_id = solver.metadata().solver_id;
    result.solver_version = solver.metadata().version;
    result.problem_id = problem.mission_id + ":run:" + std::to_string(repetition);
    result.state = state;
    result.wall_time_ms = std::chrono::duration<double, std::milli>(end - begin).count();

    if (const auto* candidate = context.candidate()) {
        result.feasibility = candidate->feasibility;
        result.route_cost = candidate->objective_score;
        result.route_nodes = candidate->route_elements.size();
    }
    return result;
}

} // namespace

BenchmarkReport run_astar_dijkstra_benchmark(const MissionProblem& problem,
                                             std::uint64_t repetitions) {
    if (problem.planning_graph == nullptr) {
        throw std::invalid_argument("benchmark requires a planning graph");
    }
    if (repetitions == 0) repetitions = 1;

    AStarSolver astar([](const PlanningNode& a, const PlanningNode& b) {
        const double dx = a.x - b.x;
        const double dy = a.y - b.y;
        return std::sqrt(dx * dx + dy * dy);
    });
    DijkstraSolver dijkstra;

    BenchmarkReport report;
    report.problem_id = problem.mission_id;
    report.results.reserve(static_cast<std::size_t>(2 * repetitions));

    for (std::uint64_t i = 1; i <= repetitions; ++i) {
        report.results.push_back(run_one(astar, problem, i));
        report.results.push_back(run_one(dijkstra, problem, i));
    }
    return report;
}

} // namespace bluesky::planning::benchmark
