#include "fleet_addressing_runtime.hpp"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include <iostream>

using namespace bluesky::operations;

int main() {
    FleetAddressingRuntime fleet;

    const FleetAddressBinding uav1{"UAV-01", "ArduPilot:MAVLink2:1:1", "S1", {1, 1}};
    const FleetAddressBinding uav2{"UAV-02", "ArduPilot:MAVLink2:2:1", "S2", {2, 1}};

    assert(fleet.registerBinding(uav1) == FleetAddressRegistrationResult::Registered);
    assert(fleet.registerBinding(uav1) == FleetAddressRegistrationResult::AlreadyRegistered);
    assert(fleet.registerBinding(uav2) == FleetAddressRegistrationResult::Registered);

    const auto byAddress = fleet.resolveAddress({2, 1});
    assert(byAddress && byAddress->vehicle_id == "UAV-02");

    const auto byVehicle = fleet.resolveVehicle("UAV-01");
    const FleetAddress expectedAddress{1, 1};
    assert(byVehicle && byVehicle->address == expectedAddress);

    const FleetAddressBinding duplicateAddress{"UAV-03", "ArduPilot:MAVLink2:3:1", "S3", {1, 1}};
    assert(fleet.registerBinding(duplicateAddress) == FleetAddressRegistrationResult::AddressCollision);

    const FleetAddressBinding duplicateVehicle{"UAV-01", "ArduPilot:MAVLink2:9:1", "S9", {9, 1}};
    assert(fleet.registerBinding(duplicateVehicle) == FleetAddressRegistrationResult::VehicleCollision);

    assert(fleet.registerBinding({"UAV-00", "SRC", "S0", {0, 1}}) == FleetAddressRegistrationResult::InvalidAddress);
    assert(fleet.registerBinding({"UAV-X", "", "S4", {4, 1}}) == FleetAddressRegistrationResult::InvalidAddress);

    assert(fleet.snapshot().size() == 2);
    assert(fleet.unregisterVehicle("UAV-01"));
    assert(!fleet.resolveAddress({1, 1}).has_value());
    assert(fleet.registerBinding(duplicateAddress) == FleetAddressRegistrationResult::Registered);

    std::cout << "fleet_addressing_runtime_test: PASS\n";
}
