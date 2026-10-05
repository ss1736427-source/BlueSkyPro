#include "candidate_comparison.hpp"
#include <algorithm>
#include <sstream>

namespace bluesky::planning {
namespace {
bool lessByObjective(const CandidateSolution& a, const CandidateSolution& b,
                     const std::vector<std::string>& priorities) {
    for (const auto& p : priorities) {
        if (p == "time") {
            if (a.estimated_time_s != b.estimated_time_s)
                return a.estimated_time_s < b.estimated_time_s;
        } else if (p == "energy") {
            if (a.estimated_energy_wh != b.estimated_energy_wh)
                return a.estimated_energy_wh < b.estimated_energy_wh;
        } else if (p == "reserve") {
            if (a.estimated_reserve_wh != b.estimated_reserve_wh)
                return a.estimated_reserve_wh > b.estimated_reserve_wh;
        } else if (p == "objective_score") {
            if (a.objective_score != b.objective_score)
                return a.objective_score < b.objective_score;
        }
    }
    return a.candidate_id < b.candidate_id;
}
}

CandidateComparisonResult CandidateComparator::compare(
    const CandidateComparisonInput& input,
    const std::string& version) {
    CandidateComparisonResult result;
    result.calculation_version = version;
    result.dependency_identity = version + "|" + input.calculation_input_version;
    std::vector<CandidateSolution> feasible;
    for (const auto& c : input.candidates) {
        if (c.feasibility == Feasibility::Feasible) feasible.push_back(c);
        else result.rejected_candidate_ids.push_back(c.candidate_id);
        result.dependency_identity += "|" + c.candidate_id + ":" + c.solver_version;
    }
    if (feasible.empty()) return result;
    std::stable_sort(feasible.begin(), feasible.end(),
        [&](const auto& a, const auto& b) {
            return lessByObjective(a, b, input.objective_priorities);
        });
    result.feasible = true;
    result.selected_candidate_id = feasible.front().candidate_id;
    return result;
}

} // namespace bluesky::planning
