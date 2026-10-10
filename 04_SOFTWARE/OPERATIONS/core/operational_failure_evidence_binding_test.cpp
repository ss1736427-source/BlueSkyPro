#include "operational_failure_evidence_binding.hpp"

#include <cassert>

int main() {
    bluesky::operations::DecisionEvidence decision;
    decision.decision_id = "decision-ph4-006";
    decision.selected_action_id = "mission-transfer";
    decision.state_transition = "SELECTED->FAILED";

    const auto evidence = bluesky::operations::bind_failure_evidence(
        decision,
        "FAILED",
        "MISSION_SEMANTIC_VERIFICATION_FAILED",
        "AUTOPILOT_ADAPTER",
        "MIS-PH4-006");

    assert(evidence.decision_id == "decision-ph4-006");
    assert(evidence.action_id == "mission-transfer");
    assert(evidence.state_transition == "SELECTED->FAILED");
    assert(evidence.result == "FAILED");
    assert(evidence.reason == "MISSION_SEMANTIC_VERIFICATION_FAILED");
    assert(evidence.source_interface == "AUTOPILOT_ADAPTER");
    assert(evidence.target_object == "MIS-PH4-006");
    assert(evidence.evidence_domain == "EC-20");

    return 0;
}
