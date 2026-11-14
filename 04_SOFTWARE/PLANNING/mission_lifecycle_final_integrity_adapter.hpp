#pragma once

#include "final_planning_integrity.hpp"
#include "mission_lifecycle.hpp"

namespace bluesky::planning {

// Converts the actual compiled-profile integrity result into a READY-gate
// attestation. A failed check or mismatched mission lineage can never attest PASS.
inline MissionLifecycleValidationEvidence makeFinalPlanningIntegrityValidationEvidence(
    const VehicleCompiledProfile& compiled,
    const std::string& expected_mission_id,
    const std::string& expected_mission_version,
    const std::string& route_id,
    const std::string& route_version,
    const std::string& calculation_input_version)
{
    const auto integrity = FinalPlanningIntegrityChecker::check(
        compiled, route_version, calculation_input_version);

    MissionLifecycleValidationEvidence evidence;
    evidence.result_id = integrity.dependency_identity;
    evidence.mission_id = compiled.mission_id;
    evidence.mission_version = compiled.mission_version;
    evidence.route_id = route_id;
    evidence.route_version = compiled.route_version;
    evidence.calculation_input_version = compiled.calculation_input_version;
    evidence.validator_id = "FinalPlanningIntegrityChecker";
    evidence.validator_version = "1";
    evidence.source_reference = integrity.dependency_identity;
    evidence.vehicle_configuration_id = compiled.configuration_version;

    const bool missionLineageMatches =
        compiled.mission_id == expected_mission_id
        && compiled.mission_version == expected_mission_version;
    evidence.status = integrity.status == FinalPlanningIntegrityStatus::Valid
        && missionLineageMatches
        ? MissionLifecycleValidationStatus::Passed
        : MissionLifecycleValidationStatus::Failed;
    return evidence;
}

} // namespace bluesky::planning
