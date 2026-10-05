#pragma once

#include "solver_contract.hpp"
#include <algorithm>
#include <limits>
#include <string>

namespace bluesky::planning {

struct CandidateComparison {
    bool admissible{false};
    double score{std::numeric_limits<double>::infinity()};
    std::string reason;
};

class SolverCandidateEvaluator {
public:
    static CandidateComparison evaluate(const CandidateSolution& candidate,
                                        double minimum_reserve_wh,
                                        double max_energy_wh,
                                        double required_quality_score = 0.0) {
        if (candidate.feasibility != Feasibility::Feasible) {
            return {false, std::numeric_limits<double>::infinity(),
                    "candidate is not fully feasible"};
        }
        if (candidate.estimated_energy_wh > max_energy_wh) {
            return {false, std::numeric_limits<double>::infinity(),
                    "energy limit exceeded"};
        }
        if (candidate.estimated_reserve_wh < minimum_reserve_wh) {
            return {false, std::numeric_limits<double>::infinity(),
                    "minimum energy reserve not satisfied"};
        }
        if (candidate.objective_score < required_quality_score) {
            return {false, std::numeric_limits<double>::infinity(),
                    "required objective quality not satisfied"};
        }
        return {true, candidate.objective_score, "admissible"};
    }

    static const CandidateSolution* best(const CandidateSolution& a,
                                         const CandidateSolution& b,
                                         double minimum_reserve_wh,
                                         double max_energy_wh,
                                         double required_quality_score = 0.0) {
        const auto ea = evaluate(a, minimum_reserve_wh, max_energy_wh, required_quality_score);
        const auto eb = evaluate(b, minimum_reserve_wh, max_energy_wh, required_quality_score);

        if (!ea.admissible) return eb.admissible ? &b : nullptr;
        if (!eb.admissible) return &a;
        return ea.score <= eb.score ? &a : &b;
    }
};

} // namespace bluesky::planning
