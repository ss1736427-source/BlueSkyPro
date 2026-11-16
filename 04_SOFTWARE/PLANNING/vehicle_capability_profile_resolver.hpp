#pragma once

#include "mission_lifecycle.hpp"
#include "model/vehicle_equipment_capability.hpp"

#include <optional>
#include <string>
#include <vector>

namespace bluesky::planning {

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
            || provenance.configuration_revision.empty())
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
