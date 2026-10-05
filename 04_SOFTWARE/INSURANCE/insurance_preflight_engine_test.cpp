#include "insurance_preflight_engine.hpp"

#include <cassert>
#include <iostream>

using namespace bluesky::insurance;

namespace {

RegulatoryRuleset ruleset(bool applicable = true) {
    RegulatoryRuleset r;
    r.jurisdiction = "RU";
    r.ruleset_id = "RU-UAS-INS";
    r.version = "2026.09";
    r.insurance_applicable = applicable;
    r.liability_mode = applicable ? LiabilityCalculationMode::MrotPerKg
                                  : LiabilityCalculationMode::None;
    r.mtom_kg = 10.0;
    r.mrot_rub = 27093.0;
    r.mrot_multiplier_per_kg = 2.0;
    r.remote_id_required = true;
    return r;
}

InsuredUav uav() {
    return {"UAV-001", "SN-001", "Reference-UAS", "RU-ACCOUNT-001",
            "OWNER-001", "OPERATOR-001", "CFG-001"};
}

InsurancePolicy policy() {
    InsurancePolicy p;
    p.policy_id = "POL-001";
    p.policy_version = "v3";
    p.insured_uav_id = "UAV-001";
    p.valid_from_epoch = 100;
    p.valid_to_epoch = 200;
    p.liability_limit_rub = 600000.0;
    p.operation_covered = true;
    p.territory_covered = true;
    p.owner_operator_condition_satisfied = true;
    p.pilot_condition_satisfied = true;
    p.technical_condition_satisfied = true;
    p.configuration_condition_satisfied = true;
    return p;
}

OperationContext operation() {
    OperationContext o;
    o.flight_record_id = "FLIGHT-001";
    o.operation_type = "COMMERCIAL";
    o.territory_id = "RU-AREA-001";
    o.pilot_id = "PILOT-001";
    o.evaluation_time_epoch = 150;
    o.remote_id_available = true;
    o.weather_within_policy_limits = true;
    return o;
}

void valid_policy_passes() {
    const auto r = InsurancePreflightEngine{}.evaluate(ruleset(), uav(), policy(), operation());
    assert(r.decision == InsuranceDecision::Pass);
    assert(r.insurance_applicable);
    assert(r.minimum_liability_rub == 541860.0);
    assert(r.snapshot.decision_text() == "PASS");
    assert(r.snapshot.flight_record_id() == "FLIGHT-001");
    assert(r.snapshot.insured_uav_id() == "UAV-001");
    assert(r.snapshot.policy_id() == "POL-001");
    assert(!r.snapshot.integrity_hash().empty());
}

void expired_policy_blocks() {
    auto p = policy(); p.valid_to_epoch = 149;
    assert(InsurancePreflightEngine{}.evaluate(ruleset(), uav(), p, operation()).decision == InsuranceDecision::Block);
}

void wrong_uav_blocks() {
    auto p = policy(); p.insured_uav_id = "UAV-999";
    assert(InsurancePreflightEngine{}.evaluate(ruleset(), uav(), p, operation()).decision == InsuranceDecision::Block);
}

void operation_outside_coverage_blocks() {
    auto p = policy(); p.operation_covered = false;
    assert(InsurancePreflightEngine{}.evaluate(ruleset(), uav(), p, operation()).decision == InsuranceDecision::Block);
}

void not_applicable_does_not_block() {
    auto r = ruleset(false);
    const auto result = InsurancePreflightEngine{}.evaluate(r, uav(), InsurancePolicy{}, operation());
    assert(!result.insurance_applicable);
    assert(result.minimum_liability_rub == 0.0);
    assert(result.decision != InsuranceDecision::Block);
}

void insufficient_limit_blocks() {
    auto p = policy(); p.liability_limit_rub = 541859.0;
    assert(InsurancePreflightEngine{}.evaluate(ruleset(), uav(), p, operation()).decision == InsuranceDecision::Block);
}

void ambiguous_requires_review() {
    auto p = policy(); p.ambiguous_coverage = true;
    assert(InsurancePreflightEngine{}.evaluate(ruleset(), uav(), p, operation()).decision == InsuranceDecision::Review);
}

void remote_id_blocks() {
    auto o = operation(); o.remote_id_available = false;
    assert(InsurancePreflightEngine{}.evaluate(ruleset(), uav(), policy(), o).decision == InsuranceDecision::Block);
}

void snapshot_is_stable() {
    const auto a = InsurancePreflightEngine{}.evaluate(ruleset(), uav(), policy(), operation());
    const auto b = InsurancePreflightEngine{}.evaluate(ruleset(), uav(), policy(), operation());
    assert(a.snapshot.integrity_hash() == b.snapshot.integrity_hash());
    assert(a.snapshot.snapshot_id() == b.snapshot.snapshot_id());
}

}  // namespace

int main() {
    valid_policy_passes();
    expired_policy_blocks();
    wrong_uav_blocks();
    operation_outside_coverage_blocks();
    not_applicable_does_not_block();
    insufficient_limit_blocks();
    ambiguous_requires_review();
    remote_id_blocks();
    snapshot_is_stable();
    std::cout << "insurance_preflight_engine_test: PASS\n";
}
