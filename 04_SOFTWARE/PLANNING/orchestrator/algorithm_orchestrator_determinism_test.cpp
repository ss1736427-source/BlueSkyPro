#include "algorithm_orchestrator.hpp"

#include <cassert>
#include <memory>
#include <string>
#include <utility>
#include <vector>

using namespace bluesky::planning;

namespace {
class Context final : public SolverContext {
public:
    explicit Context(const MissionProblem& problem) : problem_(problem) {}
    const MissionProblem& problem() const override { return problem_; }
    const ComputeBudget& budget() const override { return budget_; }
    bool cancelled() const override { return false; }
    void publish(CandidateSolution candidate) override { candidates_.push_back(std::move(candidate)); }
    const std::vector<CandidateSolution>& candidates() const override { return candidates_; }

private:
    const MissionProblem& problem_;
    ComputeBudget budget_{1000, 256, 8};
    std::vector<CandidateSolution> candidates_;
};

class EqualCandidateSolver final : public Solver {
public:
    explicit EqualCandidateSolver(std::string id) : id_(std::move(id)) {}
    SolverMetadata metadata() const override { return {id_, "test", {"point_to_point"}, false, true}; }
    bool eligible(const MissionProblem& problem) const override {
        return problem.problem_class == "point_to_point";
    }
    RunState run(SolverContext& context) override {
        CandidateSolution candidate;
        candidate.candidate_id = context.problem().mission_id + ":" + id_ + ":1";
        candidate.solver_id = id_;
        candidate.solver_version = "test";
        candidate.route_elements = {"A", "B"};
        candidate.estimated_time_s = 10.0;
        candidate.estimated_energy_wh = 5.0;
        candidate.estimated_reserve_wh = 20.0;
        candidate.objective_score = 7.0;
        candidate.feasibility = Feasibility::Feasible;
        context.publish(std::move(candidate));
        return RunState::Completed;
    }
    void cancel() override {}

private:
    std::string id_;
};
}

int main() {
    PlanningGraph graph;
    graph.nodes = {{"A", 0.0, 0.0}, {"B", 1.0, 0.0}};
    graph.edges = {{"A", "B", 1.0}};
    graph.start_node = "A";
    graph.goal_node = "B";

    MissionProblem problem;
    problem.mission_id = "ORCH-DETERMINISM-001";
    problem.problem_class = "point_to_point";
    problem.planning_graph = &graph;
    problem.objective_priorities = {"completion_time"};

    std::vector<std::unique_ptr<Solver>> solvers;
    solvers.push_back(std::make_unique<EqualCandidateSolver>("zeta"));
    solvers.push_back(std::make_unique<EqualCandidateSolver>("alpha"));

    Context context(problem);
    AlgorithmOrchestrator orchestrator(std::move(solvers));
    const auto decision = orchestrator.solve(context);

    assert(decision.feasibility == Feasibility::Feasible);
    assert(decision.selected_solver_id == "alpha");
    assert(decision.selected_candidate_id == "ORCH-DETERMINISM-001:alpha:1");
    assert(context.candidates().size() == 2);

    return 0;
}
