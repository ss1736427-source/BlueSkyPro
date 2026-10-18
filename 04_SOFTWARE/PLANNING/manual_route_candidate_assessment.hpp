#pragma once

#include "constrained_open_space.hpp"
#include "manual_route_candidate_builder.hpp"
#include "notam_prohibited_zone_validator.hpp"

#include <string>
#include <vector>

namespace bluesky::planning {

// Evaluates a structurally built manual candidate against one concrete airspace
// snapshot. This is an airspace assessment only: it does not evaluate terrain,
// obstacle clearance, wind/energy, task quality, authorization, or release readiness.
struct ManualRouteCandidateAssessment {
    bool assessed{false};
    bool candidate_structurally_valid{false};
    bool snapshot_lineage_matches{false};
    bool airspace_clear{false};
    bool flight_release_eligible{false}; // This assessment can never authorize release.
    OpenSpaceResult airspace_result;
    std::vector<std::string> errors;
};

ManualRouteCandidateAssessment assessManualRouteCandidateAirspace(
    const ManualRouteBuildResult& candidate,
    const ConstrainedEnvironmentSnapshot& environment);

// Assessment against the canonical NOTAM snapshot model. The caller must supply
// the current calculation-input version from the planning orchestration context.
struct ManualRouteNotamAssessment {
    bool assessed{false};
    bool candidate_structurally_valid{false};
    bool snapshot_lineage_matches{false};
    bool notam_clear{false};
    bool flight_release_eligible{false}; // NOTAM clearance alone never authorizes release.
    NotamValidationResult notam_result;
    std::vector<std::string> errors;
};

ManualRouteNotamAssessment assessManualRouteCandidateNotams(
    const ManualRouteBuildResult& candidate,
    const NotamSnapshot& snapshot,
    const std::string& current_calculation_input_version);

} // namespace bluesky::planning
