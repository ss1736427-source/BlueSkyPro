#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace bluesky::operations {

struct FleetAddress {
    std::uint8_t system_id{0};
    std::uint8_t component_id{0};
    bool operator==(const FleetAddress&) const = default;
};

struct FleetAddressBinding {
    std::string vehicle_id;
    std::string source_id;
    std::string session_id;
    FleetAddress address;
};

enum class FleetAddressRegistrationResult {
    Registered,
    AlreadyRegistered,
    InvalidAddress,
    VehicleCollision,
    AddressCollision
};

class FleetAddressingRuntime final {
public:
    FleetAddressRegistrationResult registerBinding(const FleetAddressBinding& binding);
    bool unregisterVehicle(const std::string& vehicle_id);
    std::optional<FleetAddressBinding> resolveAddress(FleetAddress address) const;
    std::optional<FleetAddressBinding> resolveVehicle(const std::string& vehicle_id) const;
    std::vector<FleetAddressBinding> snapshot() const;

private:
    static bool validAddress(FleetAddress address);
    std::vector<FleetAddressBinding> bindings_;
};

} // namespace bluesky::operations
