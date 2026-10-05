#include "device_identity_runtime.hpp"

#include <algorithm>

namespace bluesky::operations {

bool DeviceIdentityRuntime::validIdentity(const DeviceIdentity& identity) {
    return !identity.device_id.empty() &&
           !identity.vehicle_id.empty() &&
           !identity.autopilot_id.empty() &&
           !identity.component_id.empty() &&
           !identity.credential_fingerprint.empty();
}

DeviceIdentityRegistrationResult DeviceIdentityRuntime::registerIdentity(
    const DeviceIdentity& identity) {
    if (!validIdentity(identity)) {
        return DeviceIdentityRegistrationResult::InvalidIdentity;
    }

    for (const auto& existing : identities_) {
        if (existing.device_id == identity.device_id) {
            if (existing.vehicle_id == identity.vehicle_id &&
                existing.autopilot_id == identity.autopilot_id &&
                existing.component_id == identity.component_id &&
                existing.credential_fingerprint == identity.credential_fingerprint) {
                return DeviceIdentityRegistrationResult::AlreadyRegistered;
            }
            return DeviceIdentityRegistrationResult::IdentityCollision;
        }

        if (existing.credential_fingerprint == identity.credential_fingerprint &&
            existing.device_id != identity.device_id) {
            return DeviceIdentityRegistrationResult::CredentialCollision;
        }
    }

    identities_.push_back(identity);
    return DeviceIdentityRegistrationResult::Registered;
}

bool DeviceIdentityRuntime::setLifecycle(const std::string& device_id,
                                         DeviceIdentityLifecycle lifecycle) {
    for (auto& identity : identities_) {
        if (identity.device_id == device_id) {
            identity.lifecycle = lifecycle;
            return true;
        }
    }
    return false;
}

bool DeviceIdentityRuntime::unregisterIdentity(const std::string& device_id) {
    const auto it = std::remove_if(
        identities_.begin(), identities_.end(),
        [&](const auto& identity) { return identity.device_id == device_id; });
    if (it == identities_.end()) {
        return false;
    }
    identities_.erase(it, identities_.end());
    return true;
}

std::optional<DeviceIdentity> DeviceIdentityRuntime::resolveDevice(
    const std::string& device_id) const {
    for (const auto& identity : identities_) {
        if (identity.device_id == device_id) {
            return identity;
        }
    }
    return std::nullopt;
}

std::optional<DeviceIdentity> DeviceIdentityRuntime::resolveVehicle(
    const std::string& vehicle_id) const {
    for (const auto& identity : identities_) {
        if (identity.vehicle_id == vehicle_id) {
            return identity;
        }
    }
    return std::nullopt;
}

std::vector<DeviceIdentity> DeviceIdentityRuntime::snapshot() const {
    return identities_;
}

} // namespace bluesky::operations
