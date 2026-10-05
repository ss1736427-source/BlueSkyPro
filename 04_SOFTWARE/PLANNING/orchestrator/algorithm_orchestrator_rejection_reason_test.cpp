#include "algorithm_orchestrator.hpp"

#include <algorithm>
#include <cassert>
#include <memory>
#include <string>
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

class RejectedSolver final : public Solver {
public:
    SolverMetadata metadata() const override {
        return {"rejected", "test", {"point_to_point"}, false, true};
    }

    bool eligible(const MissionProblem& problem) const override {
        return problem.problem_class == "point_to_point";
    }

    RunState run(SolverContext& context) override {
        CandidateSolution candidate;
        candidate.candidate_id = context.problem().mission_id + ":rejected:1";
        candidate.solver_id = "rejected";
        candidate.solver_version = "test";
        candidate.route_elements = {"A", "B"};
        candidate.estimated_time_s = 10.0;
        candidate.estimated_energy_wh = 100.0;
        candidate.estimated_reserve_wh = 20.0;
        candidate.objective_score = 1.0;
        candidate.constraint_violations = {"restricted_airspace"};
        candidate.feasibility = Feasibility::Feasible;
        context.publish(std::move(candidate));
        return RunState::Completed;
    }

    void cancel() override {}
};
} // namespace

int main() {
    MissionProblem problem;
    problem.mission_id = "ORCH-REJECTION-013";
    problem.problem_class = "point_to_point";
    problem.objective_priorities = {"completion_time"};
    problem.hard_constraints = {"restricted_airspace", "operational_volume"};

    TestContext context(problem);
    CandidateConstraintValidator validator = [](const MissionProblem&, const CandidateSolution&) {
        return std::vector<std::string>{"outside_operational_volume"};
    };

    std::vector<std::unique_ptr<Solver>> solvers;
    solvers.push_back(std::make_unique<RejectedSolver>());
    AlgorithmOrchestrator orchestrator(std::move(solvers), std::move(validator));
    const auto decision = orchestrator.solve(context);

    assert(decision.feasibility == Feasibility::Infeasible);
    assert(decision.selected_solver_id.empty());
    assert(decision.selected_candidate_id.empty());
    assert(decision.rejected_candidates.size() == 1);
    assert(decision.rejected_candidates[0].candidate_id == "ORCH-REJECTION-013:rejected:1");
    assert(decision.rejected_candidates[0].solver_id == "rejected");
    const auto& violations = decision.rejected_candidates[0].violations;
    assert(std::find(violations.begin(), violations.end(), "restricted_airspace") != violations.end());
    assert(std::find(violations.begin(), violations.end(), "outside_operational_volume") != violations.end());
    assert(decision.explanation.find("все полученные кандидаты отклонены") != std::string::npos);

    return 0;
}
