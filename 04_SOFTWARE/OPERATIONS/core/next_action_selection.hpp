#pragma once

#include "authority_automation_resolution.hpp"

#include <algorithm>
#include <string>
#include <vector>

namespace bluesky::operations {

enum class SelectionStatus { None, Selected };

enum class SelectionReason { None, Executable, HumanRequired };

struct SelectionContext {
    AuthorityContext authority;
};

struct NextActionSelection {
    SelectionStatus status{SelectionStatus::None};
    const Action* action{nullptr};
    ExecutionResolution resolution{};
    SelectionReason reason{SelectionReason::None};
};

inline std::vector<const Action*> eligible_next_actions(
    const OperationalSnapshot& snapshot, const SelectionContext& context) {
    std::vector<const Action*> result;
    for (const auto* action : actionable(snapshot)) {
        const auto resolution = resolve_execution(*action, context.authority);
        if (resolution.status != ResolutionStatus::Blocked) result.push_back(action);
    }
    return result;
}

inline NextActionSelection select_next_action(
    const OperationalSnapshot& snapshot, const SelectionContext& context) {
    const auto candidates = eligible_next_actions(snapshot, context);
    if (candidates.empty()) return {};

    const auto* selected = candidates.front();
    const auto resolution = resolve_execution(*selected, context.authority);
    return {SelectionStatus::Selected, selected, resolution,
            resolution.status == ResolutionStatus::Executable
                ? SelectionReason::Executable
                : SelectionReason::HumanRequired};
}

inline std::vector<const Action*> select_parallel_actions(
    const OperationalSnapshot& snapshot, const SelectionContext& context) {
    const auto candidates = eligible_next_actions(snapshot, context);
    if (candidates.empty()) return {};

    const int top_priority = candidates.front()->priority;
    std::vector<const Action*> result;
    for (const auto* candidate : candidates) {
        if (candidate->priority != top_priority) break;
        result.push_back(candidate);
    }
    return result;
}

} // namespace bluesky::operations
