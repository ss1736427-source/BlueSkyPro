#include "mission_objective_profiles.hpp"

#include <cassert>

using namespace bluesky::operations;

int main() {
    ObjectiveProfileSelectionContext survey{"survey", false, false, false};
    assert(select_objective_profile(survey) == MissionObjectiveProfile::SurveyMapping);

    ObjectiveProfileSelectionContext urgent{"survey", false, true, false};
    assert(select_objective_profile(urgent) == MissionObjectiveProfile::TimeCriticalArrival);

    ObjectiveProfileSelectionContext bvlos{"inspection", false, false, true};
    assert(select_objective_profile(bvlos) == MissionObjectiveProfile::LongEnduranceBvlos);

    ObjectiveProfileSelectionContext multi{"survey", true, false, false};
    assert(select_objective_profile(multi) == MissionObjectiveProfile::MultiUavCoordinated);

    const auto& profile = objective_profile(MissionObjectiveProfile::Delivery);
    assert(std::string(profile.primary_objective).find("delivery") != std::string::npos);
    assert(hard_admissibility_before_objective_evaluation());

    return 0;
}
