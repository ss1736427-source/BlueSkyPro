#include "fleet_addressing_runtime.hpp"
#include <algorithm>

namespace bluesky::operations {

bool FleetAddressingRuntime::validAddress(FleetAddress address) {
    return address.system_id != 0 && address.component_id != 0;
}

FleetAddressRegistrationResult FleetAddressingRuntime::registerBinding(
    const FleetAddressBinding& binding) {
    if (binding.vehicle_id.empty() || binding.source_id.empty() ||
        binding.session_id.empty() || !validAddress(binding.address)) {
        return FleetAddressRegistrationResult::InvalidAddress;
    }

    const auto vehicle = std::find_if(bindings_.begin(), bindings_.end(),
        [&](const FleetAddressBinding& current) {
            return current.vehicle_id == binding.vehicle_id;
        });
    if (vehicle != bindings_.end()) {
        return vehicle->source_id == binding.source_id &&
               vehicle->session_id == binding.session_id &&
               vehicle->address == binding.address
            ? FleetAddressRegistrationResult::AlreadyRegistered
            : FleetAddressRegistrationResult::VehicleCollision;
    }

    const auto address = std::find_if(bindings_.begin(), bindings_.end(),
        [&](const FleetAddressBinding& current) {
            return current.address == binding.address;
        });
    if (address != bindings_.end()) {
        return FleetAddressRegistrationResult::AddressCollision;
    }

    bindings_.push_back(binding);
    return FleetAddressRegistrationResult::Registered;
}

bool FleetAddressingRuntime::unregisterVehicle(const std::string& vehicle_id) {
    const auto it = std::remove_if(bindings_.begin(), bindings_.end(),
        [&](const FleetAddressBinding& binding) { return binding.vehicle_id == vehicle_id; });
    if (it == bindings_.end()) return false;
    bindings_.erase(it, bindings_.end());
    return true;
}

std::optional<FleetAddressBinding> FleetAddressingRuntime::resolveAddress(
    FleetAddress address) const {
    const auto it = std::find_if(bindings_.begin(), bindings_.end(),
        [&](const FleetAddressBinding& binding) { return binding.address == address; });
    if (it == bindings_.end()) return std::nullopt;
    return *it;
}

std::optional<FleetAddressBinding> FleetAddressingRuntime::resolveVehicle(
    const std::string& vehicle_id) const {
    const auto it = std::find_if(bindings_.begin(), bindings_.end(),
        [&](const FleetAddressBinding& binding) { return binding.vehicle_id == vehicle_id; });
    if (it == bindings_.end()) return std::nullopt;
    return *it;
}

std::vector<FleetAddressBinding> FleetAddressingRuntime::snapshot() const {
    return bindings_;
}

} // namespace bluesky::operations
