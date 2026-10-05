#include "selected_route_set.hpp"
namespace bluesky::planning {
SelectedRouteSet SelectedRouteSetBuilder::build(
    const MissionProblem& mission, const CandidateSolution& selected,
    const std::string& selection_version, const std::string& dependency_identity) {
    SelectedRouteSet result;
    result.mission_id = mission.mission_id;
    result.mission_version = mission.mission_version;
    result.candidate_id = selected.candidate_id;
    result.route_elements = selected.route_elements;
    result.calculation_input_version = mission.environment_version;
    result.source_dependency_identity = dependency_identity;
    result.selection_version = selection_version;
    return result;
}
}
