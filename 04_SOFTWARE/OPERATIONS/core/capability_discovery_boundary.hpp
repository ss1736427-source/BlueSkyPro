#pragma once

#include "device_capability_runtime.hpp"
#include "device_identity_runtime.hpp"

#include <string>
#include <vector>

namespace bluesky::operations {

struct CapabilityDiscoveryReport {
    std::string device_id;
    std::string firmware;
    std::string protocol;
    std::string protocol_version;
    std::vector<std::string> capabilities;
};

enum class CapabilityDiscoveryResult {
    Registered,
    Updated,
    IdentityNotFound,
    IdentityRevoked,
    DeviceMismatch,
    InvalidReport
};

class CapabilityDiscoveryBoundary final {
public:
    explicit CapabilityDiscoveryBoundary(
        DeviceIdentityRuntime& identity_runtime,
        DeviceCapabilityRuntime& capability_runtime);

    CapabilityDiscoveryResult ingest(
        const CapabilityDiscoveryReport& report);

private:
    static bool valid(const CapabilityDiscoveryReport& report);

    DeviceIdentityRuntime& identity_runtime_;
    DeviceCapabilityRuntime& capability_runtime_;
};

} // namespace bluesky::operations
