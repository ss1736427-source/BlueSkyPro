#pragma once

#include "operational_decision_evidence.hpp"

#include <string>

namespace bluesky::operations {

struct OperationalFailureEvidence {
    std::string decision_id;
    std::string action_id;
    std::string state_transition;
    std::string result;
    std::string reason;
    std::string source_interface;
    std::string target_object;
    std::string evidence_domain;
};

inline OperationalFailureEvidence bind_failure_evidence(
    const DecisionEvidence& decision,
    const std::string& result,
    const std::string& reason,
    const std::string& source_interface,
    const std::string& target_object,
    const std::string& evidence_domain = "EC-20") {
    OperationalFailureEvidence evidence;
    evidence.decision_id = decision.decision_id;
    evidence.action_id = decision.selected_action_id;
    evidence.state_transition = decision.state_transition;
    evidence.result = result;
    evidence.reason = reason;
    evidence.source_interface = source_interface;
    evidence.target_object = target_object;
    evidence.evidence_domain = evidence_domain;
    return evidence;
}

} // namespace bluesky::operations
