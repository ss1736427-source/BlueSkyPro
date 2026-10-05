#pragma once

#include "universal_safety_failsafe_adapter.hpp"

#include <string>
#include <vector>

namespace bluesky::safety {

enum class ReconciliationStatus { Matched, Mismatch, Unknown, Unsupported };

enum class SafetyConstraintKind {
    Geofence,
    RallyReturnPoint,
    FailsafeAction,
    FlightModeRestriction,
    BatteryLimit,
    NavigationLoss,
    C2Loss
};

struct SafetyConstraint {
    SafetyConstraintKind kind{SafetyConstraintKind::Geofence};
    bool applicable{false};
    SafetyAction expectedAction{SafetyAction::None};
};

struct SafetyReconciliationItem {
    SafetyConstraintKind kind{SafetyConstraintKind::Geofence};
    ReconciliationStatus status{ReconciliationStatus::Unknown};
    bool releaseBlocking{false};
};

struct SafetyReconciliationResult {
    bool releaseAllowed{false};
    std::vector<SafetyReconciliationItem> items;
    std::string reason;
};

class SafetyReconciliation {
public:
    static SafetyReconciliationResult evaluate(
        const std::vector<SafetyConstraint>& constraints,
        const UniversalSafetyFailsafeAdapter& autopilotSafety);
};

} // namespace bluesky::safety
