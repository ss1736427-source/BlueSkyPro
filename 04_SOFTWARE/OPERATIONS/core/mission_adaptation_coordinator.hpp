#pragma once

#include "operational_orchestrator_contract.hpp"

#include <algorithm>
#include <string>
#include <vector>

namespace bluesky::operations {

enum class AdaptationType {
    None,
    AlternateVehicle,
    AlternatePayload,
    MultiUavAllocation,
    TaskRedistribution,
    RouteProfileChange,
    SpeedAltitudeChange,
    MissionSplit,
    MissionConsolidation,
    ScheduleChange
};

enum class AdaptationStatus { None, Proposed, RequiresHuman, Infeasible };

struct AdaptationCandidate {
    std::string id;
    AdaptationType type{AdaptationType::None};
    std::vector<std::string> affected_resource_ids;
    std::vector<std::string> affected_task_ids;
    bool within_authorized_envelope{false};
    bool safety_ok{false};
    bool regulatory_ok{false};
    bool energy_ok{false};
    bool feasible{false};
    int priority{0};
};

struct AdaptationContext {
    bool adaptation_authorized{false};
};

struct AdaptationResult {
    AdaptationStatus status{AdaptationStatus::None};
    const AdaptationCandidate* candidate{nullptr};
};

inline bool admissible(const AdaptationCandidate& candidate,
                       const AdaptationContext& context) {
    return candidate.feasible && candidate.within_authorized_envelope &&
           candidate.safety_ok && candidate.regulatory_ok && candidate.energy_ok &&
           context.adaptation_authorized;
}

inline bool mandatory_gates_ok(const AdaptationCandidate& candidate) {
    return candidate.feasible && candidate.safety_ok && candidate.regulatory_ok &&
           candidate.energy_ok;
}

inline std::vector<const AdaptationCandidate*> admissible_candidates(
    const std::vector<AdaptationCandidate>& candidates,
    const AdaptationContext& context) {
    std::vector<const AdaptationCandidate*> result;
    for (const auto& candidate : candidates) {
        if (admissible(candidate, context)) result.push_back(&candidate);
    }
    std::stable_sort(result.begin(), result.end(),
                     [](const auto* lhs, const auto* rhs) {
                         if (lhs->priority != rhs->priority)
                             return lhs->priority > rhs->priority;
                         return lhs->id < rhs->id;
                     });
    return result;
}

inline AdaptationResult select_adaptation(
    const std::vector<AdaptationCandidate>& candidates,
    const AdaptationContext& context) {
    const auto admissible_set = admissible_candidates(candidates, context);
    if (!admissible_set.empty())
        return {AdaptationStatus::Proposed, admissible_set.front()};

    for (const auto& candidate : candidates) {
        if (mandatory_gates_ok(candidate))
            return {AdaptationStatus::RequiresHuman, &candidate};
    }
    return {AdaptationStatus::Infeasible, nullptr};
}

} // namespace bluesky::operations
