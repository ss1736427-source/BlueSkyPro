#pragma once

#include "next_action_selection.hpp"

#include <string>
#include <vector>

namespace bluesky::operations {

enum class DecisionOutcome { Selected, HumanRequired, None };

struct DecisionEvidence {
    std::string decision_id;
    std::string policy_version;
    std::string input_state_version;
    std::string selected_action_id;
    std::vector<std::string> considered_action_ids;
    std::vector<std::string> constraints;
    Authority authority{Authority::None};
    AutomationLevel automation{AutomationLevel::HumanGuided};
    DecisionOutcome outcome{DecisionOutcome::None};
    std::string state_transition;
    std::string completion_evidence;
};

inline DecisionEvidence record_selection(
    const NextActionSelection& selection,
    const std::string& decision_id,
    const std::string& policy_version,
    const std::string& input_state_version,
    const std::vector<std::string>& considered_action_ids,
    const std::vector<std::string>& constraints,
    const std::string& state_transition,
    const std::string& completion_evidence = {}) {
    DecisionEvidence evidence;
    evidence.decision_id = decision_id;
    evidence.policy_version = policy_version;
    evidence.input_state_version = input_state_version;
    evidence.considered_action_ids = considered_action_ids;
    evidence.constraints = constraints;
    evidence.state_transition = state_transition;
    evidence.completion_evidence = completion_evidence;

    if (selection.action == nullptr) return evidence;

    evidence.selected_action_id = selection.action->id;
    evidence.authority = selection.resolution.authority;
    evidence.automation = selection.resolution.automation;
    evidence.outcome = selection.reason == SelectionReason::Executable
                           ? DecisionOutcome::Selected
                           : DecisionOutcome::HumanRequired;
    return evidence;
}

} // namespace bluesky::operations
