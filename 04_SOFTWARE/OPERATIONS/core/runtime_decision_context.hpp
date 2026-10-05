#pragma once

#include <string>

namespace bluesky::operations {

enum class RuntimeCoordinationMode {
    Continue,
    Recalculate,
    Replan,
    Recover,
    AwaitRuntime
};

struct RuntimeDecisionContext {
    std::string runtime_state_version;
    std::string active_mission_package_version;
    std::string actual_vehicle_state_version;
    RuntimeCoordinationMode mode{RuntimeCoordinationMode::AwaitRuntime};
    bool runtime_state_current{false};
    bool active_package_consistent{false};
    bool safety_action_required{false};
    bool energy_gate_ok{false};
    bool c2_operational{false};
};

// Runtime owns state transitions and event classification. This contract only
// validates the facts presented to the Operational Orchestrator.
inline bool decision_context_usable(const RuntimeDecisionContext& context) {
    return context.runtime_state_current &&
           !context.runtime_state_version.empty() &&
           !context.active_mission_package_version.empty() &&
           !context.actual_vehicle_state_version.empty() &&
           context.active_package_consistent;
}

// Hard runtime safety/recovery conditions are gates for ordinary orchestration.
inline bool operational_selection_allowed(const RuntimeDecisionContext& context) {
    if (!decision_context_usable(context)) {
        return false;
    }

    if (context.safety_action_required || context.mode == RuntimeCoordinationMode::Recover) {
        return false;
    }

    return context.energy_gate_ok;
}

} // namespace bluesky::operations
