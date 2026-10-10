#include "candidate_comparison.hpp"
#include <cassert>

using namespace bluesky::planning;

int main() {
    CandidateSolution energy{};
    energy.candidate_id = "A";
    energy.solver_version = "1";
    energy.feasibility = Feasibility::Feasible;
    energy.estimated_time_s = 100.0;
    energy.estimated_energy_wh = 80.0;
    energy.estimated_reserve_wh = 40.0;

    CandidateSolution fast = energy;
    fast.candidate_id = "B";
    fast.estimated_time_s = 80.0;
    fast.estimated_energy_wh = 100.0;
    fast.estimated_reserve_wh = 20.0;

    CandidateSolution rejected = energy;
    rejected.candidate_id = "C";
    rejected.feasibility = Feasibility::Infeasible;

    CandidateComparisonInput input{{energy, fast, rejected}, {"energy", "time"}, "INPUT-1"};
    const auto r = CandidateComparator::compare(input, "1.0.0");
    assert(r.feasible);
    assert(r.selected_candidate_id == "A");
    assert(r.rejected_candidate_ids.size() == 1);

    input.objective_priorities = {"time", "energy"};
    const auto fast_r = CandidateComparator::compare(input, "1.0.0");
    assert(fast_r.selected_candidate_id == "B");

    return 0;
}
