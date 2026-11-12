#include "mission_lifecycle_terrain_adapter.hpp"

#include <cassert>
#include <string>

using namespace bluesky::planning;

static Route makeRoute()
{
    Route route;
    route.lineage = {"ROUTE-DEM-1", "4", "MISSION-DEM-1", "8",
                     "terrain-test", "1", "INPUT-DEM-3"};
    route.environment.terrain_snapshot_id = "DEM-SNAPSHOT-7";
    route.constraints.minimum_clearance_m = 30.0;
    route.trajectory_geometry_complete = true;
    route.waypoints = {
        {"WP-1", {60.0, 25.0}, 80.0, true, AltitudeReference::AGL},
        {"WP-2", {60.01, 25.01}, 80.0, false, AltitudeReference::AGL}
    };
    route.segments = {{"SEG-1", "WP-1", "WP-2", 1244.0, 20.0}};
    return route;
}

static TerrainDemSnapshot makeSnapshot()
{
    TerrainDemSnapshot snapshot;
    snapshot.snapshot_id = "DEM-SNAPSHOT-7";
    snapshot.source_id = "MAPTILER-TERRAIN-RGB";
    snapshot.source_version = "terrain-rgb-v2";
    snapshot.dataset_id = "GLOBAL-DEM";
    snapshot.dataset_version = "2026-11";
    snapshot.horizontal_crs = "EPSG:4326";
    snapshot.vertical_datum = "EGM96";
    snapshot.horizontal_resolution_m = 30.0;
    snapshot.vertical_accuracy_m = 10.0;
    snapshot.coverage_complete = true;
    return snapshot;
}

static TerrainProfileValidationResult makeValidation()
{
    TerrainProfileValidationResult result;
    result.validator_id = "terrain-route-profile-validator";
    result.validator_version = "1";
    result.snapshot_id = "DEM-SNAPSHOT-7";
    result.route_id = "ROUTE-DEM-1";
    result.route_version = "4";
    result.calculation_input_version = "INPUT-DEM-3";
    result.profile_sample_count = 42;
    result.maximum_sample_gap_m = 25.0;
    result.minimum_observed_clearance_m = 45.0;
    result.complete_trajectory_coverage = true;
    result.vertical_datum_normalized = true;
    result.findings_empty = true;
    result.passed = true;
    return result;
}

int main()
{
    const auto route = makeRoute();
    const auto snapshot = makeSnapshot();
    const auto validation = makeValidation();

    const auto accepted = makeTerrainValidationEvidence(route, snapshot, validation);
    assert(accepted.status == MissionLifecycleValidationStatus::Passed);
    assert(accepted.mission_id == route.lineage.mission_id);
    assert(accepted.route_id == route.lineage.route_id);
    assert(accepted.source_reference.find("DEM-SNAPSHOT-7") != std::string::npos);

    auto wrongSnapshot = snapshot;
    wrongSnapshot.snapshot_id = "DEM-SNAPSHOT-OTHER";
    assert(makeTerrainValidationEvidence(route, wrongSnapshot, validation).status
           == MissionLifecycleValidationStatus::Failed);

    auto incompleteCoverage = snapshot;
    incompleteCoverage.coverage_complete = false;
    assert(makeTerrainValidationEvidence(route, incompleteCoverage, validation).status
           == MissionLifecycleValidationStatus::Failed);

    auto unknownDatum = snapshot;
    unknownDatum.vertical_datum.clear();
    assert(makeTerrainValidationEvidence(route, unknownDatum, validation).status
           == MissionLifecycleValidationStatus::Failed);

    auto staleRoute = validation;
    staleRoute.route_version = "3";
    assert(makeTerrainValidationEvidence(route, snapshot, staleRoute).status
           == MissionLifecycleValidationStatus::Failed);

    auto sparseProfile = validation;
    sparseProfile.profile_sample_count = 1;
    assert(makeTerrainValidationEvidence(route, snapshot, sparseProfile).status
           == MissionLifecycleValidationStatus::Failed);

    auto clearanceFailure = validation;
    clearanceFailure.minimum_observed_clearance_m = 29.9;
    assert(makeTerrainValidationEvidence(route, snapshot, clearanceFailure).status
           == MissionLifecycleValidationStatus::Failed);

    auto missingDatumNormalization = validation;
    missingDatumNormalization.vertical_datum_normalized = false;
    assert(makeTerrainValidationEvidence(route, snapshot, missingDatumNormalization).status
           == MissionLifecycleValidationStatus::Failed);

    auto missingFindingsCheck = validation;
    missingFindingsCheck.findings_empty = false;
    assert(makeTerrainValidationEvidence(route, snapshot, missingFindingsCheck).status
           == MissionLifecycleValidationStatus::Failed);

    return 0;
}
