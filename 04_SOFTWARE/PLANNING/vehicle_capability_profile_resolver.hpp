#pragma once

#include "mission_lifecycle.hpp"
#include "model/vehicle_equipment_capability.hpp"

#include <cmath>
#include <optional>
#include <string>
#include <vector>

namespace bluesky::planning {
namespace {

bool hasFinitePositiveValue(double value) noexcept
{
    return std::isfinite(value) && value > 0.0;
}

bool hasCapabilityValue(const VehicleEquipmentCapabilityProfile& profile,
                        const std::string& parameterId) noexcept
{
    const auto& flight = profile.flight;
    const auto& energy = profile.energy;
    if (parameterId == "minimum_airspeed") return hasFinitePositiveValue(flight.minimum_airspeed);
    if (parameterId == "maximum_airspeed") return hasFinitePositiveValue(flight.maximum_airspeed);
    if (parameterId == "cruise_speed") return hasFinitePositiveValue(flight.cruise_speed);
    if (parameterId == "economic_speed") return hasFinitePositiveValue(flight.economic_speed);
    if (parameterId == "climb_limit") return hasFinitePositiveValue(flight.climb_limit);
    if (parameterId == "descent_limit") return hasFinitePositiveValue(flight.descent_limit);
    if (parameterId == "turning_limit") return hasFinitePositiveValue(flight.turning_limit);
    if (parameterId == "maximum_bank_angle_rad")
        return hasFinitePositiveValue(flight.maximum_bank_angle_rad);
    if (parameterId == "maximum_lateral_acceleration_mps2")
        return hasFinitePositiveValue(flight.maximum_lateral_acceleration_mps2);
    if (parameterId == "endurance_seconds") return hasFinitePositiveValue(flight.endurance_seconds);
    if (parameterId == "wind_tolerance") return hasFinitePositiveValue(flight.wind_tolerance);
    if (parameterId == "usable_energy_wh" || parameterId == "usable_energy_model")
        return hasFinitePositiveValue(energy.usable_energy);
    if (parameterId == "battery_health")
        return std::isfinite(energy.battery_health)
            && energy.battery_health > 0.0 && energy.battery_health <= 1.0;
    if (parameterId == "battery_state")
        return std::isfinite(energy.battery_state)
            && energy.battery_state > 0.0 && energy.battery_state <= 1.0;
    if (parameterId == "reserve_requirement_wh")
        return std::isfinite(energy.reserve_requirement) && energy.reserve_requirement >= 0.0;
    return false;
}

} // namespace

struct VehicleCapabilityProfileResolutionRequest {
    std::optional<MissionExecutionMode> execution_mode;
    VehicleEquipmentCapabilityProfile profile;
    // Only parameters required by the current task/validation chain are checked.
    std::vector<std::string> required_parameter_ids;
};

struct VehicleCapabilityProfileResolution {
    bool accepted{false};
    MissionExecutionMode execution_mode{MissionExecutionMode::Physical};
    std::string vehicle_source_provenance;
    std::string release_target;
    VehicleEquipmentCapabilityProfile profile;
    std::vector<std::string> blockers;
};

class VehicleCapabilityProfileResolver final {
public:
    static VehicleCapabilityProfileResolution resolve(
        const VehicleCapabilityProfileResolutionRequest& request)
    {
        VehicleCapabilityProfileResolution result;
        result.profile = request.profile;

        if (!request.execution_mode.has_value()) {
            result.blockers.emplace_back("EXECUTION_MODE_REQUIRED");
            return result;
        }

        result.execution_mode = *request.execution_mode;
        const auto& provenance = request.profile.provenance;
        if (provenance.profile_id.empty() || provenance.profile_revision.empty())
            result.blockers.emplace_back("CAPABILITY_PROFILE_ID_AND_REVISION_REQUIRED");

        if (request.profile.identity.vehicle_type_model.empty()
            || request.profile.identity.airframe_configuration.empty())
            result.blockers.emplace_back("VEHICLE_MODEL_AND_VARIANT_REQUIRED");

        if (request.profile.configuration.configuration_version.empty()
            || provenance.configuration_id.empty()
            || provenance.configuration_revision.empty()
            || request.profile.configuration.configuration_version != provenance.configuration_revision)
            result.blockers.emplace_back("VERSIONED_VEHICLE_CONFIGURATION_REQUIRED");

        if (request.execution_mode == MissionExecutionMode::Virtual) {
            result.vehicle_source_provenance = "SIL_FIXTURE";
            result.release_target = "SIMULATOR_ONLY";
            if (provenance.source_kind != "REFERENCE_PASSPORT")
                result.blockers.emplace_back("VIRTUAL_MODE_REQUIRES_MANUFACTURER_PASSPORT_PROFILE");
            if (provenance.source_reference.empty() || provenance.source_revision.empty())
                result.blockers.emplace_back("PASSPORT_SOURCE_REFERENCE_AND_REVISION_REQUIRED");
        } else {
            result.vehicle_source_provenance = "PHYSICAL_LIVE";
            result.release_target = "PHYSICAL_UAV";
            if (provenance.source_kind != "PHYSICAL_CONFIGURATION_VERIFIED")
                result.blockers.emplace_back("PHYSICAL_MODE_REQUIRES_VERIFIED_CONFIGURATION_PROFILE");
            if (request.profile.identity.vehicle_id.empty()
                || !provenance.configuration_verified
                || !request.profile.configuration.valid_for_mission)
                result.blockers.emplace_back("PHYSICAL_VEHICLE_CONFIGURATION_NOT_VERIFIED");
            if (provenance.source_reference.empty() || provenance.source_revision.empty())
                result.blockers.emplace_back("PHYSICAL_CONFIGURATION_EVIDENCE_REFERENCE_REQUIRED");
        }

        for (const auto& required_id : request.required_parameter_ids) {
            if (required_id.empty()) {
                result.blockers.emplace_back("REQUIRED_CAPABILITY_PARAMETER_ID_EMPTY");
                continue;
            }
            const CapabilityParameterEvidence* match = nullptr;
            bool duplicate = false;
            for (const auto& evidence : provenance.parameter_evidence) {
                if (evidence.parameter_id != required_id)
                    continue;
                if (match != nullptr)
                    duplicate = true;
                match = &evidence;
            }
            if (duplicate) {
                result.blockers.emplace_back("CAPABILITY_PARAMETER_EVIDENCE_CONFLICT:" + required_id);
                continue;
            }
            if (match == nullptr || match->source_reference.empty()
                || match->source_revision.empty()) {
                result.blockers.emplace_back("CAPABILITY_PARAMETER_EVIDENCE_MISSING:" + required_id);
                continue;
            }
            if (!hasCapabilityValue(request.profile, required_id)) {
                result.blockers.emplace_back("CAPABILITY_PARAMETER_VALUE_MISSING_OR_INVALID:" + required_id);
                continue;
            }

            if (request.execution_mode == MissionExecutionMode::Virtual) {
                const bool admissibleSource =
                    match->source == CapabilityParameterSource::ManufacturerPassport
                    || match->source == CapabilityParameterSource::DerivedSimulationEstimate;
                if (!admissibleSource)
                    result.blockers.emplace_back("VIRTUAL_PARAMETER_SOURCE_NOT_ADMISSIBLE:" + required_id);
                if (match->source == CapabilityParameterSource::DerivedSimulationEstimate
                    && match->verified)
                    result.blockers.emplace_back("SIMULATION_ESTIMATE_MUST_NOT_BE_MARKED_AS_VERIFIED:" + required_id);
            } else if (match->source != CapabilityParameterSource::PhysicalConfigurationVerified
                       || !match->verified) {
                result.blockers.emplace_back("PHYSICAL_PARAMETER_NOT_VERIFIED:" + required_id);
            }
        }

        result.accepted = result.blockers.empty();
        return result;
    }
};

} // namespace bluesky::planning
