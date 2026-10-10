#include "operational_decision_evidence.hpp"

#include <cassert>

using namespace bluesky::operations;

int main() {
    OperationalSnapshot snapshot{
        {"prepare"},
        {{"release", {"prepare"}, AutomationLevel::Automatic, true, true, true, true, 100}}
    };
    SelectionContext context{{true, true, true}};
    const auto selection = select_next_action(snapshot, context);

    const auto evidence = record_selection(
        selection, "decision-001", "orchestrator-policy-1", "state-42",
        {"release"}, {"safety-ok", "regulatory-ok", "energy-ok"},
        "READY->RELEASED", "release_ack");

    assert(evidence.decision_id == "decision-001");
    assert(evidence.policy_version == "orchestrator-policy-1");
    assert(evidence.input_state_version == "state-42");
    assert(evidence.selected_action_id == "release");
    assert(evidence.considered_action_ids.size() == 1);
    assert(evidence.constraints.size() == 3);
    assert(evidence.authority == Authority::System);
    assert(evidence.automation == AutomationLevel::Automatic);
    assert(evidence.outcome == DecisionOutcome::Selected);
    assert(evidence.state_transition == "READY->RELEASED");
    assert(evidence.completion_evidence == "release_ack");

    OperationalSnapshot blocked{
        {}, {{"blocked", {"missing"}, AutomationLevel::Automatic, true, true, true, true, 100}}
    };
    const auto empty_selection = select_next_action(blocked, context);
    const auto empty_evidence = record_selection(
        empty_selection, "decision-002", "policy-1", "state-43", {"blocked"},
        {"dependency"}, "", "");
    assert(empty_evidence.outcome == DecisionOutcome::None);
    assert(empty_evidence.selected_action_id.empty());

    return 0;
}
