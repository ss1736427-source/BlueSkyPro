#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace bluesky::operations {

enum class DeviceIdentityLifecycle {
    Registered,
    Active,
    Revoked
};

struct DeviceIdentity {
    std::string device_id;
    std::string vehicle_id;
    std::string autopilot_id;
    std::string component_id;
    std::string credential_fingerprint;
    DeviceIdentityLifecycle lifecycle{DeviceIdentityLifecycle::Registered};
};

enum class DeviceIdentityRegistrationResult {
    Registered,
    AlreadyRegistered,
    InvalidIdentity,
    IdentityCollision,
    CredentialCollision
};

class DeviceIdentityRuntime final {
public:
    DeviceIdentityRegistrationResult registerIdentity(const DeviceIdentity& identity);
    bool setLifecycle(const std::string& device_id, DeviceIdentityLifecycle lifecycle);
    bool unregisterIdentity(const std::string& device_id);

    std::optional<DeviceIdentity> resolveDevice(const std::string& device_id) const;
    std::optional<DeviceIdentity> resolveVehicle(const std::string& vehicle_id) const;
    std::vector<DeviceIdentity> snapshot() const;

private:
    static bool validIdentity(const DeviceIdentity& identity);
    std::vector<DeviceIdentity> identities_;
};

} // namespace bluesky::operations
