#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace bluesky::operations {

enum class DeviceCapabilityLifecycle {
    Discovered,
    Validated
};

struct DeviceCapabilitySnapshot {
    std::string device_id;
    std::string vehicle_id;
    std::string autopilot_id;
    std::string firmware;
    std::string protocol;
    std::string protocol_version;
    DeviceCapabilityLifecycle lifecycle{DeviceCapabilityLifecycle::Discovered};
    std::vector<std::string> capabilities;
};

enum class DeviceCapabilityResult {
    Registered,
    AlreadyRegistered,
    Updated,
    InvalidSnapshot,
    NotFound
};

class DeviceCapabilityRuntime {
public:
    DeviceCapabilityResult registerSnapshot(DeviceCapabilitySnapshot snapshot);
    DeviceCapabilityResult updateSnapshot(DeviceCapabilitySnapshot snapshot);

    DeviceCapabilityResult setLifecycle(
        const std::string& device_id,
        DeviceCapabilityLifecycle lifecycle);

    bool resolve(
        const std::string& device_id,
        DeviceCapabilitySnapshot& snapshot) const;

    std::vector<DeviceCapabilitySnapshot> snapshot() const;
    std::size_t size() const;

private:
    static bool valid(const DeviceCapabilitySnapshot& snapshot);
    static void normalize(DeviceCapabilitySnapshot& snapshot);

    std::vector<DeviceCapabilitySnapshot> snapshots_;
};

} // namespace bluesky::operations
