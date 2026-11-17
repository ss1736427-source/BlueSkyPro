#pragma once
#include "mission_lifecycle.hpp"
#include <cstdint>
#include <string>
#include <vector>
namespace bluesky::planning {
enum class RegulatoryComplianceDecision { Compliant, NonCompliant, Indeterminate };
// Input contract for an authoritative regulatory rules engine. This adapter
// verifies provenance, scope and freshness; it does not interpret regulations.
struct RegulatoryComplianceAssessment {
    std::string assessment_id,mission_id,mission_version,route_id,route_version,calculation_input_version;
    std::string authority_id,rule_set_id,rule_set_version,source_reference;
    std::string permanent_airspace_snapshot_id;
    std::uint64_t evaluated_at_epoch_s{0},valid_until_epoch_s{0};
    std::size_t applicable_rule_count{0},evaluated_rule_count{0};
    bool operation_scope_complete{false};
    RegulatoryComplianceDecision decision{RegulatoryComplianceDecision::Indeterminate};
    std::vector<std::string> unresolved_findings;
};
inline MissionLifecycleValidationEvidence makeRegulatoryValidationEvidence(
    const RegulatoryComplianceAssessment& a,std::uint64_t now_epoch_s) {
    MissionLifecycleValidationEvidence e;
    e.result_id=a.assessment_id;e.mission_id=a.mission_id;e.mission_version=a.mission_version;
    e.route_id=a.route_id;e.route_version=a.route_version;e.calculation_input_version=a.calculation_input_version;
    e.validator_id=a.authority_id;e.validator_version=a.rule_set_version;e.source_reference=a.source_reference;
    e.snapshot_id=a.permanent_airspace_snapshot_id;
    const bool provenance=!a.assessment_id.empty()&&!a.authority_id.empty()&&!a.rule_set_id.empty()
        &&!a.rule_set_version.empty()&&!a.source_reference.empty()
        &&!a.permanent_airspace_snapshot_id.empty();
    const bool lineage=!a.mission_id.empty()&&!a.mission_version.empty()&&!a.route_id.empty()
        &&!a.route_version.empty()&&!a.calculation_input_version.empty();
    const bool fresh=now_epoch_s>0&&a.evaluated_at_epoch_s>0
        &&a.valid_until_epoch_s>=a.evaluated_at_epoch_s
        &&now_epoch_s>=a.evaluated_at_epoch_s&&now_epoch_s<=a.valid_until_epoch_s;
    const bool coverage=a.operation_scope_complete&&a.evaluated_rule_count>0
        &&a.evaluated_rule_count>=a.applicable_rule_count;
    const bool compliant=a.decision==RegulatoryComplianceDecision::Compliant&&a.unresolved_findings.empty();
    e.status=provenance&&lineage&&fresh&&coverage&&compliant
        ?MissionLifecycleValidationStatus::Passed:MissionLifecycleValidationStatus::Failed;
    return e;
}
} // namespace bluesky::planning
