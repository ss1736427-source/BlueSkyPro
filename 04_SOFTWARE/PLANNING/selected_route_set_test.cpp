#include "selected_route_set.hpp"
#include <cassert>
using namespace bluesky::planning;
int main() {
    MissionProblem mission;
    mission.mission_id = "M-1";
    mission.mission_version = "7";
    mission.environment_version = "ENV-3";
    CandidateSolution candidate;
    candidate.candidate_id = "C-2";
    candidate.feasibility = Feasibility::Feasible;
    candidate.route_elements = {"WP1","WP2","WP3"};
    const auto r = SelectedRouteSetBuilder::build(mission,candidate,"1.0.0","DEP-9");
    assert(r.mission_id == "M-1");
    assert(r.candidate_id == "C-2");
    assert(r.route_elements.size() == 3);
    assert(r.calculation_input_version == "ENV-3");
    return 0;
}
