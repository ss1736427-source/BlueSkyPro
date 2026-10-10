#pragma once

#include <string>
#include <vector>

namespace bluesky::planning {

struct MissionIdentity {
    std::string mission_id;
    std::string mission_version;
    std::string created_at;
    std::string updated_at;
    std::string organization_id;
    std::string operator_id;
    std::string mission_type;
    std::string operational_status;
    std::string parent_mission_id;
};

struct MissionIntent {
    std::string objective;
    std::string task_type;
};

struct TaskDefinition {
    std::string description;
};

struct ConstraintSet {
    std::vector<std::string> mandatory;
    std::vector<std::string> optimization_criteria;
};

struct EnvironmentInputReference {
    std::string kind;
    std::string source;
    std::string timestamp;
    std::string validity;
    std::string freshness;
    std::string quality;
};

struct EnvironmentSnapshot {
    std::string snapshot_id;
    std::string version;
    std::vector<EnvironmentInputReference> inputs;
};

struct VehicleAssignment {
    std::string vehicle_id;
    std::string configuration_version;
};

struct PayloadAssignment {
    std::string payload_id;
    std::string configuration_version;
};

struct RouteCandidateReference {
    std::string candidate_id;
    std::string solver_id;
    std::string solver_version;
};

struct SelectedSolutionReference {
    std::string candidate_id;
    std::string calculation_input_version;
    std::string algorithm_version;
    std::string model_version;
    std::string decision_rationale;
};

struct RegulatoryPlanReference {
    std::string plan_id;
    std::string status;
    std::string version;
};

struct AuthorizationReference {
    std::string authorization_id;
    std::string status;
    std::string version;
};

struct ExecutableMissionPackageReference {
    std::string package_id;
    std::string vehicle_id;
    std::string package_version;
};

struct ExecutionReference {
    std::string flight_reference_id;
    std::string vehicle_id;
    std::string autopilot_version;
    std::string c2_session_id;
};

struct Mission {
    MissionIdentity identity;
    MissionIntent intent;
    TaskDefinition task;
    ConstraintSet constraints;
    std::vector<EnvironmentSnapshot> environment_snapshots;
    std::vector<VehicleAssignment> vehicle_assignments;
    std::vector<PayloadAssignment> payload_assignments;
    std::vector<RouteCandidateReference> route_candidates;
    SelectedSolutionReference selected_solution;
    std::vector<RegulatoryPlanReference> regulatory_plans;
    std::vector<AuthorizationReference> authorizations;
    std::vector<ExecutableMissionPackageReference> executable_packages;
    std::vector<ExecutionReference> execution_references;
};

} // namespace bluesky::planning
