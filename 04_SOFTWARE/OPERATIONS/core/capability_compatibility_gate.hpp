#pragma once

#include <string>
#include <vector>

#include "device_capability_runtime.hpp"

namespace bluesky::operations {

struct CapabilityCompatibilityRequest {
    std::string device_id;
    std::vector<std::string> required_capabilities;
};

enum class CapabilityCompatibilityResult {
    Compatible,
    CapabilityNotFound,
    CapabilityNotValidated,
    MissingCapability,
    InvalidRequest
};

struct CapabilityCompatibilityDecision {
    CapabilityCompatibilityResult result{CapabilityCompatibilityResult::InvalidRequest};
    std::string missing_capability;
};

class CapabilityCompatibilityGate final {
public:
    explicit CapabilityCompatibilityGate(const DeviceCapabilityRuntime& runtime);

    CapabilityCompatibilityDecision evaluate(
        const CapabilityCompatibilityRequest& request) const;

private:
    const DeviceCapabilityRuntime& runtime_;
};

} // namespace bluesky::operations
