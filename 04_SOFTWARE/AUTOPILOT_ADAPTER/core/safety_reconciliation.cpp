#include "safety_reconciliation.hpp"

namespace bluesky::safety {

SafetyReconciliationResult SafetyReconciliation::evaluate(
    const std::vector<SafetyConstraint>& constraints,
    const UniversalSafetyFailsafeAdapter& autopilotSafety) {
    SafetyReconciliationResult result;
    const SafetyEvent event = autopilotSafety.readEvent();

    for (const auto& constraint : constraints) {
        if (!constraint.applicable) {
            result.items.push_back({constraint.kind, ReconciliationStatus::Matched, false});
            continue;
        }
        if (autopilotSafety.getQuality() == SafetyQuality::Unknown ||
            autopilotSafety.getState() == SafetyState::Unknown) {
            result.items.push_back({constraint.kind, ReconciliationStatus::Unknown, true});
            continue;
        }
        if (constraint.expectedAction == SafetyAction::None ||
            event.appliedAction == SafetyAction::None) {
            result.items.push_back({constraint.kind, ReconciliationStatus::Unsupported, true});
            continue;
        }
        if (constraint.expectedAction != event.appliedAction) {
            result.items.push_back({constraint.kind, ReconciliationStatus::Mismatch, true});
            continue;
        }
        result.items.push_back({constraint.kind, ReconciliationStatus::Matched, false});
    }

    result.releaseAllowed = !result.items.empty();
    for (const auto& item : result.items) {
        if (item.releaseBlocking) {
            result.releaseAllowed = false;
            result.reason = "mandatory safety reconciliation item is not matched";
            break;
        }
    }
    if (result.releaseAllowed) result.reason = "all applicable safety constraints matched";
    if (result.items.empty()) result.reason = "no applicable safety constraints were supplied";
    return result;
}

} // namespace bluesky::safety
