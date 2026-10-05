#pragma once

#include "insurance_preflight_engine.hpp"

namespace bluesky::insurance {

struct InsuranceReadinessAssessment {
    bool mandatory_insurance_applicable{false};
    bool blocks_readiness{false};
    InsuranceDecision insurance_decision{InsuranceDecision::Review};
};

class InsuranceReadinessGate {
public:
    static InsuranceReadinessAssessment assess(
        const InsurancePreflightResult& preflight) noexcept {
        InsuranceReadinessAssessment assessment;
        assessment.mandatory_insurance_applicable = preflight.insurance_applicable;
        assessment.insurance_decision = preflight.decision;
        assessment.blocks_readiness =
            assessment.mandatory_insurance_applicable &&
            (preflight.decision == InsuranceDecision::Block ||
             preflight.decision == InsuranceDecision::Review);
        return assessment;
    }
};

}  // namespace bluesky::insurance
