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

    // Every active restriction must belong to the exact environment version being
    // assessed. A matching top-level snapshot ID is insufficient if its contents
    // accidentally include stale restriction records.
    for (const auto& restriction : environment.restrictions) {
        if (!restriction.active) continue;
        if (restriction.restriction_id.empty()) {
            addError(result, "ACTIVE_RESTRICTION_ID_REQUIRED");
        }
        if (restriction.snapshot_version != environment.snapshot_version) {
            addError(result, "RESTRICTION_SNAPSHOT_VERSION_MISMATCH:"
                             + restriction.restriction_id);
        }
        // The candidate builder currently emits target altitude in AGL. Comparing
        // it directly to AMSL/flight-level restriction limits is dimensionally invalid
        // without an explicit terrain/pressure conversion, so reject those inputs here.
        if (restriction.altitude_reference != AltitudeReference::AGL) {
            addError(result, "RESTRICTION_ALTITUDE_DATUM_UNSUPPORTED:"
                             + restriction.restriction_id);
        }
    }
    if (!result.errors.empty()) {
        std::sort(result.errors.begin(), result.errors.end());
        result.errors.erase(std::unique(result.errors.begin(), result.errors.end()),
                            result.errors.end());
        return result;
    }

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

ManualRouteNotamAssessment assessManualRouteCandidateNotams(
    const ManualRouteBuildResult& candidate,
    const NotamSnapshot& snapshot,
    const std::string& current_calculation_input_version)
{
    ManualRouteNotamAssessment result;

    if (!candidate.valid) {
        result.errors.emplace_back("MANUAL_ROUTE_CANDIDATE_NOT_STRUCTURALLY_VALID");
        for (const auto& error : candidate.errors)
            result.errors.push_back("CANDIDATE_BUILD:" + error);
        return result;
    }
    result.candidate_structurally_valid = true;

    if (snapshot.snapshot_id.empty() || snapshot.source_id.empty()
        || snapshot.source_version.empty() || snapshot.evaluation_epoch_s <= 0) {
        result.errors.emplace_back("NOTAM_SNAPSHOT_LINEAGE_OR_EVALUATION_TIME_REQUIRED");
        return result;
    }
    if (candidate.route.environment.airspace_snapshot_id != snapshot.snapshot_id) {
        result.errors.emplace_back("NOTAM_SNAPSHOT_ID_MISMATCH");
        return result;
    }
    if (current_calculation_input_version.empty()
        || candidate.route.lineage.calculation_input_version
           != current_calculation_input_version) {
        result.errors.emplace_back("CALCULATION_INPUT_VERSION_MISMATCH");
        return result;
    }
    result.snapshot_lineage_matches = true;

    result.notam_result = NotamProhibitedZoneValidator::validate(candidate.route, snapshot);
    result.assessed = true;
    result.notam_clear = result.notam_result.status == RouteValidationStatus::Allowed;
    for (const auto& finding : result.notam_result.findings) {
        result.errors.push_back("NOTAM_VALIDATION_FAILED:"
            + std::to_string(static_cast<int>(finding.code))
            + ":" + finding.notam_id + ":" + finding.waypoint_id
            + ":" + finding.segment_start_waypoint_id
            + ":" + finding.segment_end_waypoint_id);
    }
    std::sort(result.errors.begin(), result.errors.end());
    result.errors.erase(std::unique(result.errors.begin(), result.errors.end()),
                        result.errors.end());
    // Intentionally no release transition: this only assesses NOTAM intersections.
    return result;
}


ManualRouteCandidateAirspaceNotamAssessment assessManualRouteCandidateAirspaceAndNotams(
    const ManualRouteBuildResult& candidate,
    const ConstrainedEnvironmentSnapshot& environment,
    const NotamSnapshot& notam_snapshot,
    const std::string& current_calculation_input_version)
{
    ManualRouteCandidateAirspaceNotamAssessment result;
    result.candidate_structurally_valid = candidate.valid;
    result.airspace = assessManualRouteCandidateAirspace(candidate, environment);
    result.notams = assessManualRouteCandidateNotams(
        candidate, notam_snapshot, current_calculation_input_version);

    result.airspace_clear = result.airspace.assessed && result.airspace.airspace_clear;
    result.notam_clear = result.notams.assessed && result.notams.notam_clear;
    result.assessed = result.airspace.assessed && result.notams.assessed;
    result.combined_clear = result.assessed && result.airspace_clear && result.notam_clear;

    result.errors.insert(result.errors.end(),
                         result.airspace.errors.begin(), result.airspace.errors.end());
    result.errors.insert(result.errors.end(),
                         result.notams.errors.begin(), result.notams.errors.end());
    if (!result.airspace.assessed && result.airspace.errors.empty())
        result.errors.emplace_back("AIRSPACE_ASSESSMENT_NOT_COMPLETED");
    if (!result.notams.assessed && result.notams.errors.empty())
        result.errors.emplace_back("NOTAM_ASSESSMENT_NOT_COMPLETED");
    std::sort(result.errors.begin(), result.errors.end());
    result.errors.erase(std::unique(result.errors.begin(), result.errors.end()),
                        result.errors.end());

    // This combined result checks only supplied snapshots. It does not establish
    // snapshot authority/freshness beyond the lineage checks, terrain/obstacles,
    // weather/energy, permissions, or overall release readiness.
    result.flight_release_eligible = false;
    return result;
}

} // namespace bluesky::planning
