#pragma once

#include "../solver_contract/solver_contract.hpp"
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace bluesky::planning {

using CandidateConstraintValidator = std::function<std::vector<std::string>(
    const MissionProblem&, const CandidateSolution&)>;

// A weather assessor must return metrics derived from a complete canonical
// trajectory and one immutable forecast snapshot. Missing/invalid results block
// candidate ranking when weather assessment is enabled.
struct CandidateWeatherAssessment {
    bool evaluated{false};
    std::string snapshot_id;
    std::string snapshot_version;
    double corrected_time_s{0.0};
    double corrected_energy_wh{0.0};
    double remaining_energy_wh{0.0};
    std::string error;
};

using CandidateWeatherAssessor = std::function<CandidateWeatherAssessment(
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
    // Every candidate that passed all active hard gates, retained with the
    // metrics used for this decision. This enables a weather-independent
    // planning pass to hand the complete admissible set to the weather stage.
    std::vector<CandidateSolution> admissible_candidates;
    std::vector<MaximumRegimeUse> selected_maximum_regime_uses;
    bool maximum_regime_warning_required{false};
    std::string operator_warning;
    bool weather_assessment_applied{false};
    std::string weather_snapshot_id;
    std::string weather_snapshot_version;
    double weather_corrected_time_s{0.0};
    double weather_corrected_energy_wh{0.0};
    double weather_remaining_energy_wh{0.0};
    std::string explanation;
};

class AlgorithmOrchestrator {
public:
    explicit AlgorithmOrchestrator(
        std::vector<std::unique_ptr<Solver>> solvers,
        CandidateConstraintValidator constraint_validator = {},
        CandidateWeatherAssessor weather_assessor = {});

    OrchestratorDecision solve(SolverContext& context);

private:
    std::vector<std::unique_ptr<Solver>> solvers_;
    CandidateConstraintValidator constraint_validator_;
    CandidateWeatherAssessor weather_assessor_;
};

} // namespace bluesky::planning
