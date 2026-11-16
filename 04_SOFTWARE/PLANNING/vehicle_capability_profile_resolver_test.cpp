#include "vehicle_capability_profile_resolver.hpp"

#include <cassert>

using namespace bluesky::planning;

namespace {

VehicleCapabilityProfileResolutionRequest baseRequest(MissionExecutionMode mode)
{
    VehicleCapabilityProfileResolutionRequest request;
    request.execution_mode = mode;
    request.profile.identity.vehicle_id = "SIM-INSTANCE-001";
    request.profile.identity.vehicle_type_model = "DJI Mavic 3 Enterprise";
    request.profile.identity.airframe_configuration = "FACTORY_VARIANT";
    request.profile.configuration.configuration_version = "factory-1";
    request.profile.flight.minimum_airspeed = 4.0;
    request.profile.flight.maximum_airspeed = 21.0;
    request.profile.energy.usable_energy = 48.0;
    request.profile.provenance.profile_id = "DJI-M3E-PASSPORT";
    request.profile.provenance.profile_revision = "2026-11-15";
    request.profile.provenance.configuration_id = "DJI-M3E-FACTORY";
    request.profile.provenance.configuration_revision = "factory-1";
    request.required_parameter_ids = {"maximum_airspeed", "usable_energy_model"};
    return request;
}

CapabilityParameterEvidence parameter(
    const std::string& id, CapabilityParameterSource source, bool verified = false)
{
    return {id, source, "controlled-source-reference", "rev-1", verified};
}

} // namespace

int main()
{
    auto virtualRequest = baseRequest(MissionExecutionMode::Virtual);
    virtualRequest.profile.provenance.source_kind = "REFERENCE_PASSPORT";
    virtualRequest.profile.provenance.source_reference = "manufacturer-spec-reference";
    virtualRequest.profile.provenance.source_revision = "2026-11-15";
    virtualRequest.profile.provenance.parameter_evidence = {
        parameter("maximum_airspeed", CapabilityParameterSource::ManufacturerPassport),
        parameter("usable_energy_model", CapabilityParameterSource::DerivedSimulationEstimate)
    };
    auto virtualResult = VehicleCapabilityProfileResolver::resolve(virtualRequest);
    assert(virtualResult.accepted);
    assert(virtualResult.vehicle_source_provenance == "SIL_FIXTURE");
    assert(virtualResult.release_target == "SIMULATOR_ONLY");

    // A passport-only profile must never qualify a physical aircraft.
    auto physicalFromPassport = virtualRequest;
    physicalFromPassport.execution_mode = MissionExecutionMode::Physical;
    auto rejectedPassport = VehicleCapabilityProfileResolver::resolve(physicalFromPassport);
    assert(!rejectedPassport.accepted);

    auto physicalRequest = baseRequest(MissionExecutionMode::Physical);
    physicalRequest.profile.identity.vehicle_id = "AIRFRAME-SERIAL-001";
    physicalRequest.profile.configuration.valid_for_mission = true;
    physicalRequest.profile.provenance.source_kind = "PHYSICAL_CONFIGURATION_VERIFIED";
    physicalRequest.profile.provenance.source_reference = "maintenance-and-configuration-record";
    physicalRequest.profile.provenance.source_revision = "record-17";
    physicalRequest.profile.provenance.configuration_verified = true;
    physicalRequest.profile.provenance.parameter_evidence = {
        parameter("maximum_airspeed", CapabilityParameterSource::PhysicalConfigurationVerified, true),
        parameter("usable_energy_model", CapabilityParameterSource::PhysicalConfigurationVerified, true)
    };
    auto physicalResult = VehicleCapabilityProfileResolver::resolve(physicalRequest);
    assert(physicalResult.accepted);
    assert(physicalResult.vehicle_source_provenance == "PHYSICAL_LIVE");
    assert(physicalResult.release_target == "PHYSICAL_UAV");

    // A provenance record cannot make a missing numeric value usable.
    auto missingNumericValue = virtualRequest;
    missingNumericValue.profile.energy.usable_energy = 0.0;
    auto missingValue = VehicleCapabilityProfileResolver::resolve(missingNumericValue);
    assert(!missingValue.accepted);

    // Missing required values fail closed; no implicit typical-value fallback.
    physicalRequest.profile.provenance.parameter_evidence.pop_back();
    auto missingParameter = VehicleCapabilityProfileResolver::resolve(physicalRequest);
    assert(!missingParameter.accepted);

    // A simulation estimate is explicitly non-verified and cannot satisfy PHYSICAL.
    physicalRequest.profile.provenance.parameter_evidence.push_back(
        parameter("usable_energy_model", CapabilityParameterSource::DerivedSimulationEstimate));
    auto simulatedEnergyForPhysical = VehicleCapabilityProfileResolver::resolve(physicalRequest);
    assert(!simulatedEnergyForPhysical.accepted);

    auto missingMode = virtualRequest;
    missingMode.execution_mode.reset();
    auto noMode = VehicleCapabilityProfileResolver::resolve(missingMode);
    assert(!noMode.accepted);
    assert(noMode.blockers.front() == "EXECUTION_MODE_REQUIRED");

    // Duplicate evidence is ambiguous and rejected.
    virtualRequest.profile.provenance.parameter_evidence.push_back(
        parameter("maximum_airspeed", CapabilityParameterSource::ManufacturerPassport));
    auto conflictingEvidence = VehicleCapabilityProfileResolver::resolve(virtualRequest);
    assert(!conflictingEvidence.accepted);

    return 0;
}
