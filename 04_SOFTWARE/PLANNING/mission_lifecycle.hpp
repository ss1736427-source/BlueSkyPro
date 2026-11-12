#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace bluesky::planning {

enum class MissionLifecycleState {
    Draft,
    Calculating,
    Validating,
    Ready,
    Submitted,
    Authorized,
    Released,
    Executing,
    Paused,
    Replanning,
    Completed,
    Aborted
};

const char *toString(MissionLifecycleState state) noexcept;

enum class MissionLifecycleValidationStatus { Missing, Passed, Failed };

struct MissionLifecycleValidationEvidence {
    std::string result_id;
    std::string mission_id;
    std::string mission_version;
    std::string route_id;
    std::string route_version;
    std::string calculation_input_version;
    std::string validator_id;
    std::string validator_version;
    std::string source_reference;
    MissionLifecycleValidationStatus status{MissionLifecycleValidationStatus::Missing};
};

struct MissionLifecycleEvidence {
    std::string mission_id;
    std::string mission_version;
    std::string route_id;
    std::string route_version;
    std::string calculation_input_version;
    std::string terrain_snapshot_id;
    std::string mission_notam_snapshot_id;
    std::string weather_snapshot_id;
    std::string notam_recheck_snapshot_id;
    std::string regulatory_submission_id;
    std::string authorization_id;
    std::string authorization_not_required_reason;
    std::string package_id;
    std::string package_route_id;
    std::string package_route_version;
    std::string package_vehicle_id;
    std::string execution_reference_id;
    std::string execution_vehicle_id;
    std::string postflight_record_id;

    bool task_definition_complete{false};
    bool mission_geometry_complete{false};
    bool vehicle_configuration_validated{false};
    bool equipment_configuration_validated{false};
    bool mission_notam_snapshot_current{false};
    bool terrain_snapshot_current{false};
    bool baseline_route_generated{false};
    bool weather_assessment_complete{false};
    bool weather_aware_route_generated{false};
    bool notam_recheck_complete{false};
    bool route_candidate_selected{false};
    MissionLifecycleValidationEvidence route_structure_validation;
    MissionLifecycleValidationEvidence trajectory_geometry_validation;
    MissionLifecycleValidationEvidence notam_validation;
    MissionLifecycleValidationEvidence terrain_validation;
    MissionLifecycleValidationEvidence vehicle_envelope_validation;
    MissionLifecycleValidationEvidence weather_validation;
    MissionLifecycleValidationEvidence energy_reserve_validation;
    MissionLifecycleValidationEvidence task_quality_validation;
    MissionLifecycleValidationEvidence regulatory_validation;
    bool operator_review_accepted{false};
    bool regulatory_submission_prepared{false};
    bool authorization_requirement_assessed{false};
    bool authorization_required{false};
    bool authorization_present{false};
    bool authorization_current{false};
    bool authorization_scope_matches{false};
    bool package_integrity_verified{false};
    bool package_matches_selected_route{false};
    bool package_vehicle_approved{false};
    bool execution_session_linked{false};
    bool execution_closed{false};
    bool postflight_data_archived{false};
};

struct MissionLifecycleEvent {
    std::uint64_t sequence{0};
    std::string mission_id;
    std::string mission_version;
    std::string parent_mission_version;
    std::string actor_id;
    std::string timestamp;
    std::string reason;
    MissionLifecycleState from{MissionLifecycleState::Draft};
    MissionLifecycleState to{MissionLifecycleState::Draft};
    std::string route_id;
    std::string route_version;
    std::string evidence_reference;
    std::vector<std::string> evidence_references;
};

struct MissionLifecycleRecord {
    std::string mission_id;
    std::string mission_version;
    MissionLifecycleState state{MissionLifecycleState::Draft};
    std::string parent_mission_version;
    std::string current_route_id;
    std::string current_route_version;
    std::string current_calculation_input_version;
    std::uint64_t next_event_sequence{1};
    std::vector<MissionLifecycleEvent> history;
};

struct MissionLifecycleTransitionResult {
    bool accepted{false};
    MissionLifecycleState state{MissionLifecycleState::Draft};
    std::vector<std::string> blockers;
};

class MissionLifecycleController {
public:
    static MissionLifecycleTransitionResult transition(
        MissionLifecycleRecord &record,
        MissionLifecycleState target,
        const MissionLifecycleEvidence &evidence,
        const std::string &actor_id,
        const std::string &timestamp,
        const std::string &reason);
};

} // namespace bluesky::planning
