#include "capability_admission_boundary.hpp"

namespace bluesky::operations {

CapabilityAdmissionBoundary::CapabilityAdmissionBoundary(
    const CapabilityCompatibilityGate& compatibility_gate)
    : compatibility_gate_(compatibility_gate)
{
}

CapabilityAdmissionDecision CapabilityAdmissionBoundary::admit(
    const CapabilityAdmissionRequest& request) const
{
    if (request.action_id.empty()) {
        return {
            CapabilityAdmissionState::Rejected,
            CapabilityCompatibilityResult::InvalidRequest,
            {},
            "ACTION_ID_REQUIRED",
            {}
        };
    }

    const auto compatibility = compatibility_gate_.evaluate({
        request.device_id,
        request.required_capabilities
    });

    if (compatibility.result != CapabilityCompatibilityResult::Compatible) {
        std::string reason;
        switch (compatibility.result) {
        case CapabilityCompatibilityResult::CapabilityNotFound:
            reason = "CAPABILITY_SNAPSHOT_NOT_FOUND";
            break;
        case CapabilityCompatibilityResult::CapabilityNotValidated:
            reason = "CAPABILITY_SNAPSHOT_NOT_VALIDATED";
            break;
        case CapabilityCompatibilityResult::MissingCapability:
            reason = "REQUIRED_CAPABILITY_MISSING";
            break;
        case CapabilityCompatibilityResult::InvalidRequest:
            reason = "CAPABILITY_REQUEST_INVALID";
            break;
        case CapabilityCompatibilityResult::Compatible:
            break;
        }

        return {
            CapabilityAdmissionState::Rejected,
            compatibility.result,
            request.action_id,
            reason,
            compatibility.missing_capability
        };
    }

    return {
        CapabilityAdmissionState::Admitted,
        compatibility.result,
        request.action_id,
        "CAPABILITY_COMPATIBLE",
        {}
    };
}

} // namespace bluesky::operations
