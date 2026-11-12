#pragma once

#include "mission_lifecycle.hpp"
#include "route_constraint_validator.hpp"

#include <string>

namespace bluesky::planning {

// Converts the canonical route-constraint validator output into a lifecycle
// attestation. The attestation is PASS only when the validator accepted the
// exact route and calculation-input revision supplied to it.
inline MissionLifecycleValidationEvidence makeRouteStructureValidationEvidence(
    const Route &route,
    const RouteValidationResult &validation)
{
    MissionLifecycleValidationEvidence evidence;
    evidence.result_id = validation.validator_id + ":" + validation.validator_version
        + ":" + route.lineage.route_id + ":" + route.lineage.route_version
        + ":" + validation.calculation_input_version;
    evidence.mission_id = route.lineage.mission_id;
    evidence.mission_version = route.lineage.mission_version;
    evidence.route_id = route.lineage.route_id;
    evidence.route_version = route.lineage.route_version;
    evidence.calculation_input_version = route.lineage.calculation_input_version;
    evidence.validator_id = validation.validator_id;
    evidence.validator_version = validation.validator_version;
    evidence.source_reference = "canonical-route:" + route.lineage.route_id
        + "@" + route.lineage.route_version;
    const bool lineageMatches = !evidence.result_id.empty()
        && !evidence.validator_id.empty() && !evidence.validator_version.empty()
        && validation.calculation_input_version == route.lineage.calculation_input_version
        && !route.lineage.mission_id.empty() && !route.lineage.mission_version.empty()
        && !route.lineage.route_id.empty() && !route.lineage.route_version.empty();
    evidence.status = validation.status == RouteValidationStatus::Allowed && lineageMatches
        ? MissionLifecycleValidationStatus::Passed
        : MissionLifecycleValidationStatus::Failed;
    return evidence;
}

} // namespace bluesky::planning
