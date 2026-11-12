#pragma once

#include "mission_lifecycle.hpp"
#include "model/flight_safety_constraints.hpp"

namespace bluesky::planning {

// Converts the actual shared dynamic-envelope validation result into a
// lifecycle attestation bound to the route and exact vehicle configuration.
inline MissionLifecycleValidationEvidence makeVehicleEnvelopeValidationEvidence(
    const Route &route,
    const FlightSafetyValidationResult &validation)
{
    MissionLifecycleValidationEvidence evidence;
    evidence.result_id = validation.validator_id + ":" + validation.validator_version
        + ":" + route.lineage.route_id + ":" + route.lineage.route_version
        + ":" + route.lineage.calculation_input_version
        + ":" + validation.configuration_id;
    evidence.mission_id = route.lineage.mission_id;
    evidence.mission_version = route.lineage.mission_version;
    evidence.route_id = route.lineage.route_id;
    evidence.route_version = route.lineage.route_version;
    evidence.calculation_input_version = route.lineage.calculation_input_version;
    evidence.validator_id = validation.validator_id;
    evidence.validator_version = validation.validator_version;
    evidence.source_reference = validation.source_reference;
    evidence.vehicle_configuration_id = validation.configuration_id;
    const bool lineageMatches = !route.lineage.mission_id.empty()
        && !route.lineage.mission_version.empty() && !route.lineage.route_id.empty()
        && !route.lineage.route_version.empty()
        && !route.lineage.calculation_input_version.empty()
        && !validation.vehicle_model_id.empty() && !validation.model_version.empty()
        && !validation.configuration_id.empty() && !validation.source_reference.empty()
        && !validation.validator_id.empty() && !validation.validator_version.empty();
    evidence.status = validation.status == FlightSafetyStatus::Allowed
        && validation.findings.empty() && lineageMatches
        ? MissionLifecycleValidationStatus::Passed
        : MissionLifecycleValidationStatus::Failed;
    return evidence;
}

} // namespace bluesky::planning
