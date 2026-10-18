#include "manual_route_candidate_assessment.hpp"

#include "route_constraint_validator.hpp"

#include <algorithm>

namespace bluesky::planning {
namespace {

void addError(ManualRouteCandidateAssessment& result, const std::string& error)
{
    result.errors.push_back(error);
}

} // namespace

ManualRouteCandidateAssessment assessManualRouteCandidateAirspace(
    const ManualRouteBuildResult& candidate,
    const ConstrainedEnvironmentSnapshot& environment)
{
    ManualRouteCandidateAssessment result;

    if (!candidate.valid) {
        addError(result, "MANUAL_ROUTE_CANDIDATE_NOT_STRUCTURALLY_VALID");
        for (const auto& error : candidate.errors)
            addError(result, "CANDIDATE_BUILD:" + error);
        return result;
    }
    result.candidate_structurally_valid = true;

    if (environment.snapshot_id.empty() || environment.snapshot_version.empty()
        || environment.calculation_input_version.empty()) {
        addError(result, "AIRSPACE_SNAPSHOT_LINEAGE_REQUIRED");
        return result;
    }
    if (!environment.complete) {
        addError(result, "AIRSPACE_SNAPSHOT_INCOMPLETE");
        return result;
    }
    if (candidate.route.environment.airspace_snapshot_id != environment.snapshot_id) {
        addError(result, "AIRSPACE_SNAPSHOT_ID_MISMATCH");
        return result;
    }
    if (candidate.route.lineage.calculation_input_version
        != environment.calculation_input_version) {
        addError(result, "CALCULATION_INPUT_VERSION_MISMATCH");
        return result;
    }
    result.snapshot_lineage_matches = true;

    const auto route_validation = RouteConstraintValidator::validate(candidate.route);
    if (!route_validation.findings.empty()) {
        for (const auto& finding : route_validation.findings)
            addError(result, "CANONICAL_ROUTE_VALIDATION_FAILED:"
                             + std::to_string(static_cast<int>(finding.code))
                             + ":" + finding.object_id);
        return result;
    }

    result.airspace_result = ConstrainedOpenSpace::evaluateRoute(environment, candidate.route);
    result.assessed = true;
    result.airspace_clear = result.airspace_result.allowed;
    if (!result.airspace_clear) {
        for (const auto& reason : result.airspace_result.blocking_restriction_ids)
            addError(result, "AIRSPACE_CHECK_BLOCKED:" + reason);
    }

    std::sort(result.errors.begin(), result.errors.end());
    result.errors.erase(std::unique(result.errors.begin(), result.errors.end()),
                        result.errors.end());
    // Deliberately never set flight_release_eligible to true here. Passing the
    // airspace check is not a substitute for terrain, weather, energy and safety gates.
    return result;
}

} // namespace bluesky::planning
