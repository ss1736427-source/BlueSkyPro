#pragma once

#include "../solver_contract/solver_contract.hpp"
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace bluesky::planning {

using CandidateConstraintValidator = std::function<std::vector<std::string>(
    const MissionProblem&, const CandidateSolution&)>;

struct CandidateRejection {
    std::string candidate_id;
    std::string solver_id;
    std::vector<std::string> violations;
};

struct OrchestratorDecision {
    Feasibility feasibility{Feasibility::Uncertain};
    std::string selected_solver_id;
    std::string selected_candidate_id;
    std::vector<std::string> considered_solvers;
    std::vector<std::string> rejected_solvers;
    std::vector<CandidateRejection> rejected_candidates;
    std::string explanation;
};

class AlgorithmOrchestrator {
public:
    explicit AlgorithmOrchestrator(
        std::vector<std::unique_ptr<Solver>> solvers,
        CandidateConstraintValidator constraint_validator = {});

    OrchestratorDecision solve(SolverContext& context);

private:
    std::vector<std::unique_ptr<Solver>> solvers_;
    CandidateConstraintValidator constraint_validator_;
};

} // namespace bluesky::planning
