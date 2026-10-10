#pragma once
#include "solver_contract/solver_contract.hpp"
#include <string>
#include <vector>
namespace bluesky::planning {
struct SelectedRouteSet {
    std::string mission_id;
    std::string mission_version;
    std::string candidate_id;
    std::vector<std::string> route_elements;
    std::string calculation_input_version;
    std::string source_dependency_identity;
    std::string selection_version;
};
struct SelectedRouteSetBuilder {
    static SelectedRouteSet build(const MissionProblem& mission,
                                  const CandidateSolution& selected,
                                  const std::string& selection_version,
                                  const std::string& dependency_identity);
};
} // namespace bluesky::planning
