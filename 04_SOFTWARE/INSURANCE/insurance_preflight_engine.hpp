#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace bluesky::insurance {

enum class InsuranceDecision { Pass, Warning, Review, Block };
enum class LiabilityCalculationMode { None, FixedRub, MrotPerKg };

struct RegulatoryRuleset {
    std::string jurisdiction;
    std::string ruleset_id;
    std::string version;
    bool insurance_applicable{false};
    LiabilityCalculationMode liability_mode{LiabilityCalculationMode::None};
    double mtom_kg{0.0};
    double mrot_rub{0.0};
    double mrot_multiplier_per_kg{0.0};
    double fixed_minimum_rub{0.0};
    bool remote_id_required{false};
};

struct InsuredUav {
    std::string uav_id;
    std::string serial_number;
    std::string model;
    std::string registration_or_accounting_id;
    std::string owner_id;
    std::string operator_id;
    std::string configuration_version;
};

struct InsurancePolicy {
    std::string policy_id;
    std::string policy_version;
    std::string insured_uav_id;
    std::uint64_t valid_from_epoch{0};
    std::uint64_t valid_to_epoch{0};
    double liability_limit_rub{0.0};
    bool operation_covered{false};
    bool territory_covered{false};
    bool owner_operator_condition_satisfied{false};
    bool pilot_condition_satisfied{false};
    bool technical_condition_satisfied{false};
    bool configuration_condition_satisfied{false};
    bool ambiguous_coverage{false};
};

struct OperationContext {
    std::string flight_record_id;
    std::string operation_type;
    std::string territory_id;
    std::string pilot_id;
    std::uint64_t evaluation_time_epoch{0};
    bool remote_id_available{false};
    bool weather_within_policy_limits{true};
};

struct CheckFinding {
    std::string code;
    std::string detail;
    bool blocking{false};
    bool review_required{false};
    bool warning{false};
};

class InsurancePreflightSnapshot {
public:
    InsurancePreflightSnapshot() = default;
    const std::string& snapshot_id() const noexcept { return snapshot_id_; }
    const std::string& flight_record_id() const noexcept { return flight_record_id_; }
    const std::string& insured_uav_id() const noexcept { return insured_uav_id_; }
    const std::string& policy_id() const noexcept { return policy_id_; }
    const std::string& policy_version() const noexcept { return policy_version_; }
    const std::string& ruleset_id() const noexcept { return ruleset_id_; }
    const std::string& ruleset_version() const noexcept { return ruleset_version_; }
    const std::string& decision_text() const noexcept { return decision_text_; }
    const std::string& operation_type() const noexcept { return operation_type_; }
    const std::string& territory_id() const noexcept { return territory_id_; }
    const std::string& pilot_id() const noexcept { return pilot_id_; }
    const std::string& uav_configuration_version() const noexcept { return uav_configuration_version_; }
    std::uint64_t evaluation_time_epoch() const noexcept { return evaluation_time_epoch_; }
    bool remote_id_available() const noexcept { return remote_id_available_; }
    double minimum_liability_rub() const noexcept { return minimum_liability_rub_; }
    double policy_limit_rub() const noexcept { return policy_limit_rub_; }
    const std::vector<CheckFinding>& findings() const noexcept { return findings_; }
    const std::string& integrity_hash() const noexcept { return integrity_hash_; }
private:
    friend class InsurancePreflightEngine;
    std::string snapshot_id_;
    std::string flight_record_id_;
    std::string insured_uav_id_;
    std::string policy_id_;
    std::string policy_version_;
    std::string ruleset_id_;
    std::string ruleset_version_;
    std::string decision_text_;
    std::string operation_type_;
    std::string territory_id_;
    std::string pilot_id_;
    std::string uav_configuration_version_;
    std::uint64_t evaluation_time_epoch_{0};
    bool remote_id_available_{false};
    double minimum_liability_rub_{0.0};
    double policy_limit_rub_{0.0};
    std::vector<CheckFinding> findings_;
    std::string integrity_hash_;
};

struct InsurancePreflightResult {
    bool insurance_applicable{false};
    InsuranceDecision decision{InsuranceDecision::Review};
    double minimum_liability_rub{0.0};
    std::vector<CheckFinding> findings;
    InsurancePreflightSnapshot snapshot;
};

class InsurancePreflightEngine {
public:
    InsurancePreflightResult evaluate(
        const RegulatoryRuleset& ruleset,
        const InsuredUav& uav,
        const InsurancePolicy& policy,
        const OperationContext& operation) const;
    static const char* decision_name(InsuranceDecision decision) noexcept;
private:
    static double calculate_minimum_liability(const RegulatoryRuleset& ruleset);
    static std::string calculate_integrity_hash(const InsurancePreflightSnapshot& snapshot);
};

}  // namespace bluesky::insurance
