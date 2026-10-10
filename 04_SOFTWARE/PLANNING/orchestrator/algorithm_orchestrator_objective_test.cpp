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

class ObjectiveSolver final : public Solver {
public:
    ObjectiveSolver(std::string id, double time_s, double energy_wh, double reserve_wh, double score)
        : id_(std::move(id)), time_s_(time_s), energy_wh_(energy_wh), reserve_wh_(reserve_wh), score_(score) {}
    SolverMetadata metadata() const override { return {id_, "test", {"point_to_point"}, false, true}; }
    bool eligible(const MissionProblem& problem) const override { return problem.problem_class == "point_to_point"; }
    RunState run(SolverContext& context) override {
        CandidateSolution candidate;
        candidate.candidate_id = context.problem().mission_id + ":" + id_ + ":1";
        candidate.solver_id = id_;
        candidate.solver_version = "test";
        candidate.route_elements = {"A", "B"};
        candidate.estimated_time_s = time_s_;
        candidate.estimated_energy_wh = energy_wh_;
        candidate.estimated_reserve_wh = reserve_wh_;
        candidate.objective_score = score_;
        candidate.feasibility = Feasibility::Feasible;
        context.publish(std::move(candidate));
        return RunState::Completed;
    }
    void cancel() override {}
private:
    std::string id_;
    double time_s_;
    double energy_wh_;
    double reserve_wh_;
    double score_;
};

std::vector<std::unique_ptr<Solver>> make_solvers() {
    std::vector<std::unique_ptr<Solver>> solvers;
    solvers.push_back(std::make_unique<ObjectiveSolver>("a", 10.0, 8.0, 20.0, 1.0));
    solvers.push_back(std::make_unique<ObjectiveSolver>("b", 20.0, 5.0, 10.0, 9.0));
    solvers.push_back(std::make_unique<ObjectiveSolver>("c", 30.0, 5.0, 30.0, 9.0));
    solvers.push_back(std::make_unique<ObjectiveSolver>("d", 40.0, 5.0, 10.0, 0.0));
    return solvers;
}

MissionProblem base_problem(const std::string& id, std::vector<std::string> priorities) {
    MissionProblem problem;
    problem.mission_id = id;
    problem.problem_class = "point_to_point";
    problem.objective_priorities = std::move(priorities);
    return problem;
}
}

int main() {
    // Primary energy objective: lower energy wins even when time and score are worse.
    {
        auto problem = base_problem("ORCH-ENERGY-001", {"energy"});
        Context context(problem);
        AlgorithmOrchestrator orchestrator(make_solvers());
        const auto decision = orchestrator.solve(context);
        assert(decision.feasibility == Feasibility::Feasible);
        assert(decision.selected_solver_id == "b");
    }

    // Primary reserve objective: higher reserve wins even when time is worse.
    {
        auto problem = base_problem("ORCH-RESERVE-002", {"reserve"});
        Context context(problem);
        AlgorithmOrchestrator orchestrator(make_solvers());
        const auto decision = orchestrator.solve(context);
        assert(decision.feasibility == Feasibility::Feasible);
        assert(decision.selected_solver_id == "c");
    }

    // Lexicographic priorities: energy decides first; reserve only resolves an energy tie.
    {
        auto problem = base_problem("ORCH-LEX-003", {"energy", "reserve"});
        Context context(problem);
        AlgorithmOrchestrator orchestrator(make_solvers());
        const auto decision = orchestrator.solve(context);
        assert(decision.feasibility == Feasibility::Feasible);
        assert(decision.selected_solver_id == "c");
        assert(decision.selected_candidate_id == "ORCH-LEX-003:c:1");
    }

    // Declared priorities are exhaustive: objective_score must not become an implicit fallback.
    // b and d tie on all declared objectives, while d has a better score; deterministic ID wins.
    {
        auto problem = base_problem("ORCH-TIE-004", {"energy", "reserve"});
        Context context(problem);
        AlgorithmOrchestrator orchestrator(make_solvers());
        const auto decision = orchestrator.solve(context);
        assert(decision.feasibility == Feasibility::Feasible);
        assert(decision.selected_solver_id == "b");
        assert(decision.selected_candidate_id == "ORCH-TIE-004:b:1");
    }

    // Unsupported objective priorities must not silently fall back to an arbitrary ranking.
    {
        auto problem = base_problem("ORCH-INVALID-005", {"unknown_priority"});
        Context context(problem);
        AlgorithmOrchestrator orchestrator(make_solvers());
        const auto decision = orchestrator.solve(context);
        assert(decision.feasibility == Feasibility::Uncertain);
        assert(decision.selected_solver_id.empty());
        assert(decision.selected_candidate_id.empty());
        assert(decision.explanation.find("неподдерживаемый приоритет") != std::string::npos);
    }

    // Missing priorities are also uncertain: the orchestrator must not invent a default objective.
    {
        auto problem = base_problem("ORCH-NO-PRIORITY-006", {});
        Context context(problem);
        AlgorithmOrchestrator orchestrator(make_solvers());
        const auto decision = orchestrator.solve(context);
        assert(decision.feasibility == Feasibility::Uncertain);
        assert(decision.selected_solver_id.empty());
        assert(decision.selected_candidate_id.empty());
        assert(decision.explanation.find("не определены приоритеты") != std::string::npos);
    }

    // Mandatory constraints are validated independently of the solver's self-report.
    {
        auto problem = base_problem("ORCH-CONSTRAINT-007", {"completion_time"});
        problem.hard_constraints = {"restricted_airspace"};
        Context context(problem);
        AlgorithmOrchestrator orchestrator(
            make_solvers(),
            [](const MissionProblem& p, const CandidateSolution& candidate) {
                if (!p.hard_constraints.empty() && candidate.solver_id == "a") {
                    return std::vector<std::string>{"restricted_airspace"};
                }
                return std::vector<std::string>{};
            });
        const auto decision = orchestrator.solve(context);
        assert(decision.feasibility == Feasibility::Feasible);
        assert(decision.selected_solver_id == "b");
        assert(decision.selected_candidate_id == "ORCH-CONSTRAINT-007:b:1");
        assert(decision.rejected_candidates.size() == 1);
        assert(decision.rejected_candidates[0].candidate_id == "ORCH-CONSTRAINT-007:a:1");
        assert(decision.rejected_candidates[0].violations.size() == 1);
        assert(decision.rejected_candidates[0].violations[0] == "restricted_airspace");
    }

    return 0;
}
