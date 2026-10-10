#pragma once

#include <algorithm>
#include <string>
#include <vector>

namespace bluesky::operations {

enum class AutomationLevel { Automatic, HumanGuided, HumanDecision };
enum class ActionStatus { Blocked, Actionable };

enum class Gate { None, Safety, Regulatory, Authorization, Energy, Dependency };

struct Action {
    std::string id;
    std::vector<std::string> prerequisites;
    AutomationLevel automation{AutomationLevel::HumanGuided};
    bool authorized{false};
    bool safe{true};
    bool regulatory_ok{true};
    bool energy_ok{true};
    int priority{0};
};

struct OperationalSnapshot {
    std::vector<std::string> completed_actions;
    std::vector<Action> actions;
};

inline bool contains(const std::vector<std::string>& values, const std::string& value) {
    return std::find(values.begin(), values.end(), value) != values.end();
}

inline bool prerequisites_met(const Action& action, const OperationalSnapshot& snapshot) {
    return std::all_of(action.prerequisites.begin(), action.prerequisites.end(),
                       [&](const auto& id) { return contains(snapshot.completed_actions, id); });
}

inline ActionStatus evaluate(const Action& action, const OperationalSnapshot& snapshot) {
    if (!prerequisites_met(action, snapshot) || !action.authorized || !action.safe ||
        !action.regulatory_ok || !action.energy_ok) {
        return ActionStatus::Blocked;
    }
    return ActionStatus::Actionable;
}

inline std::vector<const Action*> actionable(const OperationalSnapshot& snapshot) {
    std::vector<const Action*> result;
    for (const auto& action : snapshot.actions) {
        if (evaluate(action, snapshot) == ActionStatus::Actionable) result.push_back(&action);
    }
    std::stable_sort(result.begin(), result.end(), [](const auto* a, const auto* b) {
        if (a->priority != b->priority) return a->priority > b->priority;
        return a->id < b->id;
    });
    return result;
}

inline const Action* select_next(const OperationalSnapshot& snapshot) {
    const auto candidates = actionable(snapshot);
    return candidates.empty() ? nullptr : candidates.front();
}

} // namespace bluesky::operations
