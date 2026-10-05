#pragma once

#include "operational_orchestrator_contract.hpp"

#include <algorithm>
#include <string>
#include <vector>

namespace bluesky::operations {

struct ReadinessAction {
    Action action;
    std::string owner;
    bool completed{false};
    bool available{true};
    bool requires_human{false};
};

struct ReadinessSnapshot {
    std::vector<ReadinessAction> actions;
};

inline ActionStatus readiness_status(const ReadinessAction& action,
                                     const OperationalSnapshot& operational) {
    if (action.completed || !action.available) return ActionStatus::Blocked;
    return evaluate(action.action, operational);
}

inline std::vector<const ReadinessAction*> actionable_readiness(
    const ReadinessSnapshot& readiness, const OperationalSnapshot& operational) {
    std::vector<const ReadinessAction*> result;
    for (const auto& action : readiness.actions) {
        if (readiness_status(action, operational) == ActionStatus::Actionable) {
            result.push_back(&action);
        }
    }
    std::stable_sort(result.begin(), result.end(), [](const auto* a, const auto* b) {
        if (a->action.priority != b->action.priority) {
            return a->action.priority > b->action.priority;
        }
        return a->action.id < b->action.id;
    });
    return result;
}

inline std::vector<const ReadinessAction*> parallel_ready_set(
    const ReadinessSnapshot& readiness, const OperationalSnapshot& operational) {
    const auto candidates = actionable_readiness(readiness, operational);
    if (candidates.empty()) return {};

    const int top_priority = candidates.front()->action.priority;
    std::vector<const ReadinessAction*> result;
    for (const auto* candidate : candidates) {
        if (candidate->action.priority != top_priority) break;
        result.push_back(candidate);
    }
    return result;
}

} // namespace bluesky::operations
