#include "insurance_preflight_engine.hpp"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>

namespace bluesky::insurance {
namespace {

void add_block(std::vector<CheckFinding>& f, const char* code, const std::string& detail) {
    f.push_back({code, detail, true, false, false});
}
void add_review(std::vector<CheckFinding>& f, const char* code, const std::string& detail) {
    f.push_back({code, detail, false, true, false});
}
void add_warning(std::vector<CheckFinding>& f, const char* code, const std::string& detail) {
    f.push_back({code, detail, false, false, true});
}
std::string bit(bool value) { return value ? "1" : "0"; }

}  // namespace

const char* InsurancePreflightEngine::decision_name(InsuranceDecision decision) noexcept {
    switch (decision) {
    case InsuranceDecision::Pass: return "PASS";
    case InsuranceDecision::Warning: return "WARNING";
    case InsuranceDecision::Review: return "REVIEW";
    case InsuranceDecision::Block: return "BLOCK";
    }
    return "REVIEW";
}

double InsurancePreflightEngine::calculate_minimum_liability(const RegulatoryRuleset& r) {
    switch (r.liability_mode) {
    case LiabilityCalculationMode::None:
        return 0.0;
    case LiabilityCalculationMode::FixedRub:
        return std::max(0.0, r.fixed_minimum_rub);
    case LiabilityCalculationMode::MrotPerKg:
        return std::ceil(std::max(0.0, r.mtom_kg) *
                         std::max(0.0, r.mrot_rub) *
                         std::max(0.0, r.mrot_multiplier_per_kg));
    }
    return 0.0;
}

std::string InsurancePreflightEngine::calculate_integrity_hash(
    const InsurancePreflightSnapshot& s) {
    std::ostringstream canonical;
    canonical << s.snapshot_id_ << '|' << s.flight_record_id_ << '|'
              << s.insured_uav_id_ << '|' << s.policy_id_ << '|'
              << s.policy_version_ << '|' << s.ruleset_id_ << '|'
              << s.ruleset_version_ << '|' << s.operation_type_ << '|'
              << s.territory_id_ << '|' << s.pilot_id_ << '|'
              << s.uav_configuration_version_ << '|' << s.evaluation_time_epoch_ << '|'
              << bit(s.remote_id_available_) << '|' << std::fixed << std::setprecision(2)
              << s.minimum_liability_rub_ << '|' << s.policy_limit_rub_ << '|'
              << s.decision_text_;
    for (const auto& f : s.findings_) {
        canonical << '|' << f.code << '|' << f.detail << '|'
                  << bit(f.blocking) << bit(f.review_required) << bit(f.warning);
    }
    constexpr std::uint64_t offset = 14695981039346656037ULL;
    constexpr std::uint64_t prime = 1099511628211ULL;
    std::uint64_t hash = offset;
    for (const unsigned char ch : canonical.str()) {
        hash ^= ch;
        hash *= prime;
    }
    std::ostringstream out;
    out << std::hex << std::setw(16) << std::setfill('0') << hash;
    return out.str();
}

InsurancePreflightResult InsurancePreflightEngine::evaluate(
    const RegulatoryRuleset& ruleset,
    const InsuredUav& uav,
    const InsurancePolicy& policy,
    const OperationContext& operation) const {
    InsurancePreflightResult result;
    result.insurance_applicable = ruleset.insurance_applicable;
    result.minimum_liability_rub = calculate_minimum_liability(ruleset);
    auto& f = result.findings;

    if (uav.uav_id.empty())
        add_block(f, "UAV-ID-MISSING", "INSURED_UAV identity is missing.");

    if (policy.policy_id.empty()) {
        if (ruleset.insurance_applicable)
            add_block(f, "POLICY-MISSING", "A policy is required but no policy is supplied.");
        else
            add_warning(f, "POLICY-NOT-REQUIRED", "Insurance is not applicable under the supplied ruleset.");
    }

    if (!ruleset.insurance_applicable) {
        add_warning(f, "INSURANCE-NOT-APPLICABLE",
                    "The current regulatory ruleset does not require insurance for this operation.");
    } else {
        if (policy.insured_uav_id != uav.uav_id)
            add_block(f, "POLICY-UAV-MISMATCH", "Policy does not identify the selected INSURED_UAV.");
        if (operation.evaluation_time_epoch < policy.valid_from_epoch ||
            operation.evaluation_time_epoch > policy.valid_to_epoch)
            add_block(f, "POLICY-EXPIRED-OR-INACTIVE", "Policy is not active at the evaluation time.");
        if (!policy.operation_covered)
            add_block(f, "OPERATION-NOT-COVERED", "Operation is outside the policy coverage.");
        if (!policy.territory_covered)
            add_block(f, "TERRITORY-NOT-COVERED", "Operation territory is outside the policy coverage.");
        if (!policy.owner_operator_condition_satisfied)
            add_block(f, "OWNER-OPERATOR-CONDITION", "Owner/operator policy conditions are not satisfied.");
        if (!policy.pilot_condition_satisfied)
            add_block(f, "PILOT-CONDITION", "Pilot policy conditions are not satisfied.");
        if (!policy.technical_condition_satisfied)
            add_block(f, "TECHNICAL-CONDITION", "Technical policy conditions are not satisfied.");
        if (!policy.configuration_condition_satisfied)
            add_block(f, "CONFIGURATION-CONDITION", "Configuration policy conditions are not satisfied.");
        if (policy.ambiguous_coverage)
            add_review(f, "COVERAGE-AMBIGUOUS", "Coverage conditions cannot be resolved deterministically.");
        if (policy.liability_limit_rub + 1e-9 < result.minimum_liability_rub)
            add_block(f, "LIABILITY-LIMIT-INSUFFICIENT",
                      "Policy liability limit is below the applicable minimum.");
    }

    if (ruleset.remote_id_required && !operation.remote_id_available)
        add_block(f, "REMOTE-ID-MISSING", "Required remote identification is not available.");
    if (!operation.weather_within_policy_limits)
        add_block(f, "WEATHER-OUTSIDE-POLICY", "Current conditions are outside supplied policy limits.");
    if (uav.registration_or_accounting_id.empty())
        add_review(f, "UAV-REGISTRATION-MISSING", "Registration/accounting identity is unavailable.");
    if (uav.configuration_version.empty())
        add_review(f, "UAV-CONFIGURATION-MISSING", "UAV configuration version is unavailable.");
    if (operation.flight_record_id.empty())
        add_block(f, "FLIGHT-RECORD-MISSING",
                  "Insurance Preflight Snapshot cannot be linked to a Flight Record.");

    bool block = false, review = false, warning = false;
    for (const auto& finding : f) {
        block |= finding.blocking;
        review |= finding.review_required;
        warning |= finding.warning;
    }
    result.decision = block ? InsuranceDecision::Block :
                       review ? InsuranceDecision::Review :
                       warning ? InsuranceDecision::Warning :
                       InsuranceDecision::Pass;

    InsurancePreflightSnapshot s;
    s.snapshot_id_ = "INS-SNAPSHOT-" + operation.flight_record_id;
    s.flight_record_id_ = operation.flight_record_id;
    s.insured_uav_id_ = uav.uav_id;
    s.policy_id_ = policy.policy_id;
    s.policy_version_ = policy.policy_version;
    s.ruleset_id_ = ruleset.ruleset_id;
    s.ruleset_version_ = ruleset.version;
    s.decision_text_ = decision_name(result.decision);
    s.operation_type_ = operation.operation_type;
    s.territory_id_ = operation.territory_id;
    s.pilot_id_ = operation.pilot_id;
    s.uav_configuration_version_ = uav.configuration_version;
    s.evaluation_time_epoch_ = operation.evaluation_time_epoch;
    s.remote_id_available_ = operation.remote_id_available;
    s.minimum_liability_rub_ = result.minimum_liability_rub;
    s.policy_limit_rub_ = policy.liability_limit_rub;
    s.findings_ = f;
    s.integrity_hash_ = calculate_integrity_hash(s);
    result.snapshot = s;
    return result;
}

}  // namespace bluesky::insurance
