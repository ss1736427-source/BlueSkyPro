#include "capability_compatibility_gate.hpp"

#include <algorithm>

namespace bluesky::operations {

namespace {

std::string normalize(const std::string& value)
{
    return value;
}

bool contains(
    const std::vector<std::string>& values,
    const std::string& value)
{
    return std::binary_search(values.begin(), values.end(), value);
}

} // namespace

CapabilityCompatibilityGate::CapabilityCompatibilityGate(
    const DeviceCapabilityRuntime& runtime)
    : runtime_(runtime)
{
}

CapabilityCompatibilityDecision CapabilityCompatibilityGate::evaluate(
    const CapabilityCompatibilityRequest& request) const
{
    if (request.device_id.empty() || request.required_capabilities.empty()) {
        return {CapabilityCompatibilityResult::InvalidRequest, {}};
    }

    std::vector<std::string> required;
    required.reserve(request.required_capabilities.size());

    for (const auto& capability : request.required_capabilities) {
        if (capability.empty()) {
            return {CapabilityCompatibilityResult::InvalidRequest, {}};
        }
        required.push_back(normalize(capability));
    }

    std::sort(required.begin(), required.end());
    required.erase(
        std::unique(required.begin(), required.end()),
        required.end());

    DeviceCapabilitySnapshot snapshot;
    if (!runtime_.resolve(request.device_id, snapshot)) {
        return {CapabilityCompatibilityResult::CapabilityNotFound, {}};
    }

    if (snapshot.lifecycle != DeviceCapabilityLifecycle::Validated) {
        return {CapabilityCompatibilityResult::CapabilityNotValidated, {}};
    }

    std::vector<std::string> capabilities = snapshot.capabilities;
    std::sort(capabilities.begin(), capabilities.end());

    for (const auto& capability : required) {
        if (!contains(capabilities, capability)) {
            return {
                CapabilityCompatibilityResult::MissingCapability,
                capability
            };
        }
    }

    return {CapabilityCompatibilityResult::Compatible, {}};
}

} // namespace bluesky::operations
