#pragma once

#include "mission_lifecycle.hpp"
#include "model/route_model.hpp"

#include <cmath>
#include <string>

namespace bluesky::planning {

// Provider-neutral metadata for one immutable DEM snapshot. The actual DEM
// acquisition/decoding service owns these values; the lifecycle adapter only
// accepts a snapshot after its profile validator has checked route coverage.
struct TerrainDemSnapshot {
    std::string snapshot_id;
    std::string source_id;
    std::string source_version;
    std::string dataset_id;
    std::string dataset_version;
    std::string horizontal_crs;
    std::string vertical_datum;
    double horizontal_resolution_m{0.0};
    double vertical_accuracy_m{0.0};
    bool coverage_complete{false};
};

// Output of the authoritative route-vs-DEM profile validator. A PASS must
// represent samples along the complete flown trajectory, not just waypoints.
struct TerrainProfileValidationResult {
    std::string validator_id;
    std::string validator_version;
    std::string snapshot_id;
    std::string route_id;
    std::string route_version;
    std::string calculation_input_version;
    std::size_t profile_sample_count{0};
    double maximum_sample_gap_m{0.0};
    double minimum_observed_clearance_m{0.0};
    bool complete_trajectory_coverage{false};
    bool vertical_datum_normalized{false};
    bool findings_empty{false};
    bool passed{false};
};

inline MissionLifecycleValidationEvidence makeTerrainValidationEvidence(
    const Route &route,
    const TerrainDemSnapshot &snapshot,
    const TerrainProfileValidationResult &validation)
{
    MissionLifecycleValidationEvidence evidence;
    evidence.mission_id = route.lineage.mission_id;
    evidence.mission_version = route.lineage.mission_version;
    evidence.route_id = route.lineage.route_id;
    evidence.route_version = route.lineage.route_version;
    evidence.calculation_input_version = route.lineage.calculation_input_version;
    evidence.validator_id = validation.validator_id;
    evidence.validator_version = validation.validator_version;
    evidence.source_reference = snapshot.source_id + "@" + snapshot.source_version
        + ";dataset:" + snapshot.dataset_id + "@" + snapshot.dataset_version
        + ";snapshot:" + snapshot.snapshot_id
        + ";crs:" + snapshot.horizontal_crs
        + ";vertical-datum:" + snapshot.vertical_datum;

    const bool routeLineageValid = !route.lineage.mission_id.empty()
        && !route.lineage.mission_version.empty()
        && !route.lineage.route_id.empty()
        && !route.lineage.route_version.empty()
        && !route.lineage.calculation_input_version.empty();
    const bool snapshotIdentityValid = !snapshot.snapshot_id.empty()
        && !snapshot.source_id.empty() && !snapshot.source_version.empty()
        && !snapshot.dataset_id.empty() && !snapshot.dataset_version.empty()
        && snapshot.horizontal_crs == "EPSG:4326"
        && !snapshot.vertical_datum.empty()
        && snapshot.horizontal_resolution_m > 0.0
        && std::isfinite(snapshot.horizontal_resolution_m)
        && snapshot.vertical_accuracy_m >= 0.0
        && std::isfinite(snapshot.vertical_accuracy_m)
        && snapshot.coverage_complete
        && route.environment.terrain_snapshot_id == snapshot.snapshot_id;
    const bool profileValid = validation.passed
        && validation.findings_empty
        && validation.complete_trajectory_coverage
        && validation.vertical_datum_normalized
        && validation.profile_sample_count >= 2
        && std::isfinite(validation.maximum_sample_gap_m)
        && validation.maximum_sample_gap_m > 0.0
        && std::isfinite(validation.minimum_observed_clearance_m)
        && validation.minimum_observed_clearance_m >= route.constraints.minimum_clearance_m
        && validation.snapshot_id == snapshot.snapshot_id
        && validation.route_id == route.lineage.route_id
        && validation.route_version == route.lineage.route_version
        && validation.calculation_input_version == route.lineage.calculation_input_version
        && !validation.validator_id.empty()
        && !validation.validator_version.empty();

    evidence.result_id = validation.validator_id + ":" + validation.validator_version
        + ":" + validation.route_id + "@" + validation.route_version
        + ":" + validation.calculation_input_version
        + ":" + validation.snapshot_id;
    evidence.status = routeLineageValid && snapshotIdentityValid && profileValid
        && !evidence.source_reference.empty()
        ? MissionLifecycleValidationStatus::Passed
        : MissionLifecycleValidationStatus::Failed;
    return evidence;
}

} // namespace bluesky::planning
