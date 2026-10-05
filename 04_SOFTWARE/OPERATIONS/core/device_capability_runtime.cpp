#include "device_capability_runtime.hpp"

#include <algorithm>

namespace bluesky::operations {

bool DeviceCapabilityRuntime::valid(const DeviceCapabilitySnapshot& snapshot) {
    if (snapshot.device_id.empty()) {
        return false;
    }
    for (const auto& capability : snapshot.capabilities) {
        if (capability.empty()) {
            return false;
        }
    }
    return true;
}

void DeviceCapabilityRuntime::normalize(DeviceCapabilitySnapshot& snapshot) {
    std::sort(snapshot.capabilities.begin(), snapshot.capabilities.end());
    snapshot.capabilities.erase(
        std::unique(snapshot.capabilities.begin(), snapshot.capabilities.end()),
        snapshot.capabilities.end());
}

DeviceCapabilityResult DeviceCapabilityRuntime::registerSnapshot(
    DeviceCapabilitySnapshot snapshot) {
    if (!valid(snapshot)) {
        return DeviceCapabilityResult::InvalidSnapshot;
    }

    normalize(snapshot);

    const auto it = std::find_if(
        snapshots_.begin(),
        snapshots_.end(),
        [&](const DeviceCapabilitySnapshot& current) {
            return current.device_id == snapshot.device_id;
        });

    if (it != snapshots_.end()) {
        if (it->vehicle_id == snapshot.vehicle_id &&
            it->autopilot_id == snapshot.autopilot_id &&
            it->firmware == snapshot.firmware &&
            it->protocol == snapshot.protocol &&
            it->protocol_version == snapshot.protocol_version &&
            it->lifecycle == snapshot.lifecycle &&
            it->capabilities == snapshot.capabilities) {
            return DeviceCapabilityResult::AlreadyRegistered;
        }
        return DeviceCapabilityResult::InvalidSnapshot;
    }

    snapshots_.push_back(std::move(snapshot));
    std::sort(
        snapshots_.begin(),
        snapshots_.end(),
        [](const auto& left, const auto& right) {
            return left.device_id < right.device_id;
        });
    return DeviceCapabilityResult::Registered;
}

DeviceCapabilityResult DeviceCapabilityRuntime::updateSnapshot(
    DeviceCapabilitySnapshot snapshot) {
    if (!valid(snapshot)) {
        return DeviceCapabilityResult::InvalidSnapshot;
    }

    normalize(snapshot);

    const auto it = std::find_if(
        snapshots_.begin(),
        snapshots_.end(),
        [&](const DeviceCapabilitySnapshot& current) {
            return current.device_id == snapshot.device_id;
        });

    if (it == snapshots_.end()) {
        return DeviceCapabilityResult::NotFound;
    }

    *it = std::move(snapshot);
    std::sort(
        snapshots_.begin(),
        snapshots_.end(),
        [](const auto& left, const auto& right) {
            return left.device_id < right.device_id;
        });
    return DeviceCapabilityResult::Updated;
}

DeviceCapabilityResult DeviceCapabilityRuntime::setLifecycle(
    const std::string& device_id,
    DeviceCapabilityLifecycle lifecycle) {
    const auto it = std::find_if(
        snapshots_.begin(),
        snapshots_.end(),
        [&](const DeviceCapabilitySnapshot& current) {
            return current.device_id == device_id;
        });

    if (it == snapshots_.end()) {
        return DeviceCapabilityResult::NotFound;
    }

    it->lifecycle = lifecycle;
    return DeviceCapabilityResult::Updated;
}

bool DeviceCapabilityRuntime::resolve(
    const std::string& device_id,
    DeviceCapabilitySnapshot& snapshot) const {
    const auto it = std::find_if(
        snapshots_.begin(),
        snapshots_.end(),
        [&](const DeviceCapabilitySnapshot& current) {
            return current.device_id == device_id;
        });

    if (it == snapshots_.end()) {
        return false;
    }

    snapshot = *it;
    return true;
}

std::vector<DeviceCapabilitySnapshot> DeviceCapabilityRuntime::snapshot() const {
    return snapshots_;
}

std::size_t DeviceCapabilityRuntime::size() const {
    return snapshots_.size();
}

} // namespace bluesky::operations
