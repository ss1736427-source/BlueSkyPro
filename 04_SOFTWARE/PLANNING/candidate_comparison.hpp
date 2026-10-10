#pragma once

#include "solver_contract/solver_contract.hpp"
#include <string>
#include <vector>

namespace bluesky::planning {

struct CandidateComparisonInput {
    std::vector<CandidateSolution> candidates;
    std::vector<std::string> objective_priorities;
    std::string calculation_input_version;
};

struct CandidateComparisonResult {
    bool feasible{false};
    std::string selected_candidate_id;
    std::string calculation_version;
    std::string dependency_identity;
    std::vector<std::string> rejected_candidate_ids;
};

class CandidateComparator {
public:
    static CandidateComparisonResult compare(
        const CandidateComparisonInput& input,
        const std::string& calculation_version);
};

} // namespace bluesky::planning
