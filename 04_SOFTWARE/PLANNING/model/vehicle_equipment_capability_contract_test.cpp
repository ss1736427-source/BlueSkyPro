#include "vehicle_equipment_capability.hpp"

#include <cassert>
#include <type_traits>

using namespace bluesky::planning;

int main() {
    static_assert(std::is_default_constructible_v<VehicleEquipmentCapabilityProfile>);
    static_assert(std::is_same_v<decltype(VehicleEquipmentCapabilityProfile::identity), VehicleIdentity>);
    static_assert(std::is_same_v<decltype(VehicleEquipmentCapabilityProfile::flight), FlightCapability>);
    static_assert(std::is_same_v<decltype(VehicleEquipmentCapabilityProfile::energy), EnergyCapability>);
    static_assert(std::is_same_v<decltype(VehicleEquipmentCapabilityProfile::propulsion_resource), PropulsionResourceCapability>);
    static_assert(std::is_same_v<decltype(VehicleEquipmentCapabilityProfile::configuration), VehicleConfiguration>);

    VehicleEquipmentCapabilityProfile profile;
    profile.identity.vehicle_id = "UAV-001";
    profile.identity.autopilot_type = "generic";
    profile.flight.gnss_denied_recovery_capable = true;
    profile.energy.degradation_coefficient = 0.95;
    profile.configuration.configuration_version = "1";
    profile.configuration.valid_for_mission = true;

    assert(profile.identity.vehicle_id == "UAV-001");
    assert(profile.energy.degradation_coefficient == 0.95);
    assert(profile.configuration.valid_for_mission);
    assert(profile.flight.gnss_denied_recovery_capable);

    return 0;
}
