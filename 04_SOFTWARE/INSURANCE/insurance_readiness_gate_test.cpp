#include "insurance_readiness_gate.hpp"

#include <cassert>
#include <iostream>

using namespace bluesky::insurance;

int main() {
    InsurancePreflightResult pass;
    pass.decision = InsuranceDecision::Pass;
    pass.insurance_applicable = true;
    pass.minimum_liability_rub = 541860.0;
    auto a = InsuranceReadinessGate::assess(pass);
    assert(a.mandatory_insurance_applicable);
    assert(!a.blocks_readiness);

    InsurancePreflightResult warning;
    warning.decision = InsuranceDecision::Warning;
    warning.insurance_applicable = false;
    warning.minimum_liability_rub = 0.0;
    auto b = InsuranceReadinessGate::assess(warning);
    assert(!b.mandatory_insurance_applicable);
    assert(!b.blocks_readiness);

    InsurancePreflightResult review;
    review.decision = InsuranceDecision::Review;
    review.insurance_applicable = true;
    review.minimum_liability_rub = 541860.0;
    auto c = InsuranceReadinessGate::assess(review);
    assert(c.mandatory_insurance_applicable);
    assert(c.blocks_readiness);

    InsurancePreflightResult block;
    block.decision = InsuranceDecision::Block;
    block.insurance_applicable = true;
    block.minimum_liability_rub = 541860.0;
    auto d = InsuranceReadinessGate::assess(block);
    assert(d.mandatory_insurance_applicable);
    assert(d.blocks_readiness);

    std::cout << "insurance_readiness_gate_test: PASS\n";
}
