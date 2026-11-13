#pragma once

#include "mission_lifecycle.hpp"
#include "notam_prohibited_zone_validator.hpp"

namespace bluesky::planning {

// Converts the final, post-weather NOTAM gate result into lifecycle evidence.
// A PASS requires the validator to accept the exact route and the exact
// airspace snapshot already bound to that route.
inline MissionLifecycleValidationEvidence makeNotamValidationEvidence(
    const Route &route,
    const NotamSnapshot &snapshot,
    const NotamValidationResult &validation)
{
    MissionLifecycleValidationEvidence evidence;
    evidence.result_id = validation.validator_id + ":" + validation.validator_version
        + ":" + route.lineage.route_id + ":" + route.lineage.route_version
        + ":" + validation.snapshot_id;
    evidence.mission_id = route.lineage.mission_id;
    evidence.mission_version = route.lineage.mission_version;
    evidence.route_id = route.lineage.route_id;
    evidence.route_version = route.lineage.route_version;
    evidence.calculation_input_version = route.lineage.calculation_input_version;
    evidence.validator_id = validation.validator_id;
    evidence.validator_version = validation.validator_version;
    evidence.source_reference = snapshot.source_id + "@" + snapshot.source_version;
    evidence.snapshot_id = validation.snapshot_id;
    const bool lineageMatches = !route.lineage.mission_id.empty()
        && !route.lineage.mission_version.empty() && !route.lineage.route_id.empty()
        && !route.lineage.route_version.empty()
        && !route.lineage.calculation_input_version.empty()
        && !snapshot.source_id.empty() && !snapshot.source_version.empty()
        && !snapshot.snapshot_id.empty()
        && validation.snapshot_id == snapshot.snapshot_id
        && !evidence.source_reference.empty()
        && validation.snapshot_id == route.environment.airspace_snapshot_id
        && !validation.validator_id.empty() && !validation.validator_version.empty();
    evidence.status = validation.status == RouteValidationStatus::Allowed
        && validation.findings.empty() && lineageMatches
        ? MissionLifecycleValidationStatus::Passed
        : MissionLifecycleValidationStatus::Failed;
    return evidence;
}

} // namespace bluesky::planning
