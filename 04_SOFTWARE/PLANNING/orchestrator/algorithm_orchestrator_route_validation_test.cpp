#include "algorithm_orchestrator.hpp"

#include <cassert>
#include <memory>
#include <string>
#include <utility>
#include <vector>

using namespace bluesky::planning;

namespace {

class TestContext final : public SolverContext {
public:
    explicit TestContext(const MissionProblem& problem) : problem_(problem) {}
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

class RouteSolver final : public Solver {
public:
    RouteSolver(std::string id, std::vector<std::string> route)
        : id_(std::move(id)), route_(std::move(route)) {}
    SolverMetadata metadata() const override { return {id_, "test", {"point_to_point"}, false, true}; }
    bool eligible(const MissionProblem& problem) const override {
        return problem.problem_class == "point_to_point";
    }
    RunState run(SolverContext& context) override {
        CandidateSolution candidate;
        candidate.candidate_id = context.problem().mission_id + ":" + id_ + ":1";
        candidate.solver_id = id_;
        candidate.solver_version = "test";
        candidate.route_elements = route_;
        candidate.estimated_time_s = 5.0;
        candidate.estimated_energy_wh = 10.0;
        candidate.estimated_reserve_wh = 20.0;
        candidate.objective_score = id_ == "valid" ? 10.0 : 1.0;
        candidate.feasibility = Feasibility::Feasible;
        context.publish(std::move(candidate));
        return RunState::Completed;
    }
    void cancel() override {}
private:
    std::string id_;
    std::vector<std::string> route_;
};

} // namespace

int main() {
    PlanningGraph graph;
    graph.nodes = {{"A", 0, 0}, {"B", 1, 0}, {"C", 0, 1}};
    graph.edges = {{"A", "B", 5.0}, {"A", "C", 2.0}};
    graph.start_node = "A";
    graph.goal_node = "B";

    MissionProblem problem;
    problem.mission_id = "ORCH-ROUTE-VALIDATION-001";
    problem.problem_class = "point_to_point";
    problem.planning_graph = &graph;
    problem.objective_priorities = {"minimum_cost"};

    TestContext context(problem);
    std::vector<std::unique_ptr<Solver>> solvers;
    solvers.push_back(std::make_unique<RouteSolver>("invalid", std::vector<std::string>{"A", "C", "B"}));
    solvers.push_back(std::make_unique<RouteSolver>("valid", std::vector<std::string>{"A", "B"}));

    AlgorithmOrchestrator orchestrator(std::move(solvers));
    const auto decision = orchestrator.solve(context);

    assert(decision.feasibility == Feasibility::Feasible);
    assert(decision.selected_solver_id == "valid");
    assert(decision.selected_candidate_id == "ORCH-ROUTE-VALIDATION-001:valid:1");
    assert(context.candidates().size() == 2);

    return 0;
}
