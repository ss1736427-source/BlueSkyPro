#pragma once

#include <string>
#include <vector>

#include "capability_compatibility_gate.hpp"

namespace bluesky::operations {

struct CapabilityAdmissionRequest {
    std::string device_id;
    std::string action_id;
    std::vector<std::string> required_capabilities;
};

enum class CapabilityAdmissionState {
    Admitted,
    Rejected
};

struct CapabilityAdmissionDecision {
    CapabilityAdmissionState state{CapabilityAdmissionState::Rejected};
    CapabilityCompatibilityResult compatibility{
        CapabilityCompatibilityResult::InvalidRequest};
    std::string action_id;
    std::string reason;
    std::string missing_capability;
};

class CapabilityAdmissionBoundary final {
public:
    explicit CapabilityAdmissionBoundary(
        const CapabilityCompatibilityGate& compatibility_gate);

    CapabilityAdmissionDecision admit(
        const CapabilityAdmissionRequest& request) const;

private:
    const CapabilityCompatibilityGate& compatibility_gate_;
};

} // namespace bluesky::operations
