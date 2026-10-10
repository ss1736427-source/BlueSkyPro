#pragma once

#include "next_action_selection.hpp"
#include "operational_decision_evidence.hpp"
#include "runtime_decision_context.hpp"

#include <string>
#include <vector>

namespace bluesky::operations {

enum class PipelineStatus { None, Selected, HumanRequired, RuntimeBlocked };

struct OrchestratorPipelineContext {
    SelectionContext selection;
    RuntimeDecisionContext runtime;
    std::string policy_version;
    std::string input_state_version;
};

struct OrchestratorPipelineResult {
    PipelineStatus status{PipelineStatus::None};
    NextActionSelection selection{};
    DecisionEvidence evidence{};
};

inline OrchestratorPipelineResult run_orchestrator_pipeline(
    const OperationalSnapshot& snapshot,
    const OrchestratorPipelineContext& context) {
    OrchestratorPipelineResult result{};
    result.evidence.policy_version = context.policy_version;
    result.evidence.input_state_version = context.input_state_version;

    if (!operational_selection_allowed(context.runtime)) {
        result.status = PipelineStatus::RuntimeBlocked;
        result.evidence.outcome = DecisionOutcome::None;
        result.evidence.state_transition = "runtime-gated";
        return result;
    }

    const auto candidates = eligible_next_actions(snapshot, context.selection);
    for (const auto* candidate : candidates) {
        result.evidence.considered_action_ids.push_back(candidate->id);
    }

    result.selection = select_next_action(snapshot, context.selection);
    if (result.selection.status == SelectionStatus::None) {
        result.status = PipelineStatus::None;
        result.evidence.outcome = DecisionOutcome::None;
        result.evidence.state_transition = "no-selection";
        return result;
    }

    result.evidence.selected_action_id = result.selection.action->id;
    result.evidence.authority = result.selection.resolution.authority;
    result.evidence.automation = result.selection.resolution.automation;

    if (result.selection.reason == SelectionReason::Executable) {
        result.status = PipelineStatus::Selected;
        result.evidence.outcome = DecisionOutcome::Selected;
        result.evidence.state_transition = "action-selected";
    } else {
        result.status = PipelineStatus::HumanRequired;
        result.evidence.outcome = DecisionOutcome::HumanRequired;
        result.evidence.state_transition = "human-required";
    }

    return result;
}

} // namespace bluesky::operations
