#pragma once

#include "constrained_open_space.hpp"
#include "manual_route_candidate_builder.hpp"

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

} // namespace bluesky::planning
