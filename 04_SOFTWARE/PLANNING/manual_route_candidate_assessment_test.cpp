#include "manual_route_candidate_assessment.hpp"

#include <cassert>
#include <string>

using namespace bluesky::planning;

namespace {

Mission missionFixture()
{
    Mission mission;
    mission.identity.mission_id = "MISSION-MANUAL-001";
    mission.identity.mission_version = "7";
    mission.mission_mode = "MANUAL";
    mission.selected_template_ids = {"MT-05"};
    return mission;
}

MissionGeometryObject geometryFixture()
{
    return {
        "GEO-CORRIDOR-01",
        "CORRIDOR",
        "EPSG:4326",
        {
            {30.0000, 59.0000, 40.0, std::string("AGL")},
            {30.0100, 59.0050, 45.0, std::string("AGL")},
            {30.0200, 59.0100, 40.0, std::string("AGL")}
        },
        {"MT-05"},
        2
    };
}

ManualRouteBuildRequirements requirementsFixture()
{
    return {
        "manual-line-route-generator", "1", "INPUT-7",
        "TERRAIN-42", "AIRSPACE-17", "WEATHER-21", "WIND-21",
        20.0, 80.0, 10.0, 8.0
    };
}

ConstrainedEnvironmentSnapshot environmentFixture()
{
    ConstrainedEnvironmentSnapshot environment;
    environment.snapshot_id = "AIRSPACE-17";
    environment.snapshot_version = "AS-17-v3";
    environment.calculation_input_version = "INPUT-7";
    environment.complete = true;
    return environment;
}

template <typename Assessment>
bool hasError(const Assessment& result, const std::string& error)
{
    for (const auto& value : result.errors)
        if (value == error) return true;
    return false;
}

} // namespace

int main()
{
    const auto candidate = buildManualRouteCandidate(
        missionFixture(), geometryFixture(), requirementsFixture());
    assert(candidate.valid);
    assert(!candidate.flight_release_eligible);

    auto environment = environmentFixture();
    const auto clear = assessManualRouteCandidateAirspace(candidate, environment);
    assert(clear.assessed);
    assert(clear.candidate_structurally_valid);
    assert(clear.snapshot_lineage_matches);
    assert(clear.airspace_clear);
    assert(!clear.flight_release_eligible);
    assert(clear.errors.empty());

    // Exercise the actual NOTAM snapshot contract, including snapshot lineage,
    // evaluation time, active-zone intersection and calculation-input freshness.
    NotamSnapshot notamSnapshot;
    notamSnapshot.snapshot_id = "AIRSPACE-17";
    notamSnapshot.source_id = "AUTH-NOTAM-TEST";
    notamSnapshot.source_version = "NOTAM-v3";
    notamSnapshot.evaluation_epoch_s = 1000;
    const auto notamClear = assessManualRouteCandidateNotams(
        candidate, notamSnapshot, "INPUT-7");
    assert(notamClear.assessed);
    assert(notamClear.snapshot_lineage_matches);
    assert(notamClear.notam_clear);
    assert(!notamClear.flight_release_eligible);

    NotamZone prohibited;
    prohibited.notam_id = "NOTAM-CANONICAL-001";
    prohibited.restriction_type = NotamRestrictionType::Prohibited;
    prohibited.geometry_type = NotamGeometryType::Circle;
    prohibited.center = {59.005, 30.010};
    prohibited.radius_m = 300.0;
    prohibited.lower_altitude_m = 0.0;
    prohibited.upper_altitude_m = 100.0;
    prohibited.valid_from_epoch_s = 900;
    prohibited.valid_to_epoch_s = 1100;
    prohibited.altitude_reference = NotamAltitudeReference::AGL;
    auto notamBlockedSnapshot = notamSnapshot;
    notamBlockedSnapshot.zones.push_back(prohibited);
    const auto notamBlocked = assessManualRouteCandidateNotams(
        candidate, notamBlockedSnapshot, "INPUT-7");
    assert(notamBlocked.assessed);
    assert(!notamBlocked.notam_clear);
    assert(!notamBlocked.flight_release_eligible);

    const auto staleNotamInputs = assessManualRouteCandidateNotams(
        candidate, notamSnapshot, "INPUT-OLD");
    assert(!staleNotamInputs.assessed);
    assert(hasError(staleNotamInputs, "CALCULATION_INPUT_VERSION_MISMATCH"));

    auto missingNotamLineage = notamSnapshot;
    missingNotamLineage.source_version.clear();
    const auto noNotamLineage = assessManualRouteCandidateNotams(
        candidate, missingNotamLineage, "INPUT-7");
    assert(!noNotamLineage.assessed);
    assert(hasError(noNotamLineage, "NOTAM_SNAPSHOT_LINEAGE_OR_EVALUATION_TIME_REQUIRED"));

    auto restricted = environment;
    SpatialRestriction noFly;
    noFly.restriction_id = "NOTAM-TEST-001";
    noFly.source_id = "TEST-FIXTURE";
    noFly.snapshot_version = restricted.snapshot_version;
    noFly.geometry_type = RestrictionGeometryType::Circle;
    noFly.center = {59.005, 30.010};
    noFly.radius_m = 300.0;
    noFly.altitude_reference = AltitudeReference::AGL;
    noFly.minimum_altitude_m = 0.0;
    noFly.maximum_altitude_m = 100.0;
    noFly.active = true;
    restricted.restrictions.push_back(noFly);

    const auto blocked = assessManualRouteCandidateAirspace(candidate, restricted);
    assert(blocked.assessed);
    assert(!blocked.airspace_clear);
    assert(!blocked.flight_release_eligible);
    assert(hasError(blocked, "AIRSPACE_CHECK_BLOCKED:NOTAM-TEST-001"));

    auto staleRestriction = environment;
    auto oldNoFly = noFly;
    oldNoFly.snapshot_version = "AS-OLD";
    staleRestriction.restrictions.push_back(oldNoFly);
    const auto staleRestrictionResult =
        assessManualRouteCandidateAirspace(candidate, staleRestriction);
    assert(!staleRestrictionResult.assessed);
    assert(hasError(staleRestrictionResult,
                    "RESTRICTION_SNAPSHOT_VERSION_MISMATCH:NOTAM-TEST-001"));

    auto unknownDatum = environment;
    auto unknownRestriction = noFly;
    unknownRestriction.altitude_reference = AltitudeReference::Unknown;
    unknownDatum.restrictions.push_back(unknownRestriction);
    const auto unknownDatumResult =
        assessManualRouteCandidateAirspace(candidate, unknownDatum);
    assert(!unknownDatumResult.assessed);
    assert(hasError(unknownDatumResult,
                    "RESTRICTION_ALTITUDE_DATUM_UNSUPPORTED:NOTAM-TEST-001"));

    auto amslDatum = environment;
    auto amslRestriction = noFly;
    amslRestriction.altitude_reference = AltitudeReference::AMSL;
    amslDatum.restrictions.push_back(amslRestriction);
    const auto amslDatumResult =
        assessManualRouteCandidateAirspace(candidate, amslDatum);
    assert(!amslDatumResult.assessed);
    assert(hasError(amslDatumResult,
                    "RESTRICTION_ALTITUDE_DATUM_UNSUPPORTED:NOTAM-TEST-001"));

    auto staleSnapshot = environment;
    staleSnapshot.snapshot_id = "AIRSPACE-OLD";
    const auto stale = assessManualRouteCandidateAirspace(candidate, staleSnapshot);
    assert(!stale.assessed);
    assert(!stale.snapshot_lineage_matches);
    assert(hasError(stale, "AIRSPACE_SNAPSHOT_ID_MISMATCH"));

    auto staleInputs = environment;
    staleInputs.calculation_input_version = "INPUT-OLD";
    const auto mismatchedInputs = assessManualRouteCandidateAirspace(candidate, staleInputs);
    assert(!mismatchedInputs.assessed);
    assert(!mismatchedInputs.snapshot_lineage_matches);
    assert(hasError(mismatchedInputs, "CALCULATION_INPUT_VERSION_MISMATCH"));

    auto incomplete = environment;
    incomplete.complete = false;
    const auto incompleteResult = assessManualRouteCandidateAirspace(candidate, incomplete);
    assert(!incompleteResult.assessed);
    assert(hasError(incompleteResult, "AIRSPACE_SNAPSHOT_INCOMPLETE"));

    auto missingLineage = environment;
    missingLineage.snapshot_version.clear();
    const auto missing = assessManualRouteCandidateAirspace(candidate, missingLineage);
    assert(!missing.assessed);
    assert(hasError(missing, "AIRSPACE_SNAPSHOT_LINEAGE_REQUIRED"));

    auto invalidCandidate = candidate;
    invalidCandidate.valid = false;
    invalidCandidate.errors = {"EXPLICIT_TARGET_ALTITUDE_REQUIRED:0"};
    const auto invalid = assessManualRouteCandidateAirspace(invalidCandidate, environment);
    assert(!invalid.assessed);
    assert(!invalid.candidate_structurally_valid);
    assert(hasError(invalid, "MANUAL_ROUTE_CANDIDATE_NOT_STRUCTURALLY_VALID"));
    assert(hasError(invalid, "CANDIDATE_BUILD:EXPLICIT_TARGET_ALTITUDE_REQUIRED:0"));

    const auto combinedClear = assessManualRouteCandidateAirspaceAndNotams(
        candidate, environment, notamSnapshot, "INPUT-7");
    assert(combinedClear.assessed);
    assert(combinedClear.candidate_structurally_valid);
    assert(combinedClear.airspace_clear);
    assert(combinedClear.notam_clear);
    assert(combinedClear.combined_clear);
    assert(!combinedClear.flight_release_eligible);
    assert(combinedClear.errors.empty());

    const auto combinedNotamBlocked = assessManualRouteCandidateAirspaceAndNotams(
        candidate, environment, notamBlockedSnapshot, "INPUT-7");
    assert(combinedNotamBlocked.assessed);
    assert(combinedNotamBlocked.airspace_clear);
    assert(!combinedNotamBlocked.notam_clear);
    assert(!combinedNotamBlocked.combined_clear);
    assert(!combinedNotamBlocked.flight_release_eligible);

    const auto combinedStaleInputs = assessManualRouteCandidateAirspaceAndNotams(
        candidate, environment, notamSnapshot, "INPUT-OLD");
    assert(!combinedStaleInputs.assessed);
    assert(!combinedStaleInputs.combined_clear);
    assert(!combinedStaleInputs.flight_release_eligible);
    assert(hasError(combinedStaleInputs.notams, "CALCULATION_INPUT_VERSION_MISMATCH"));

    return 0;
}
