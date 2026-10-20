#pragma once

#include <optional>
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

struct MissionGeometryVertex {
    double longitude_deg = 0.0;
    double latitude_deg = 0.0;
    std::optional<double> altitude_m;
    std::optional<std::string> altitude_reference;
};

struct MissionGeometryObject {
    std::string geometry_id;
    std::string geometry_type;
    std::string coordinate_reference_system = "EPSG:4326";
    std::vector<MissionGeometryVertex> coordinates;
    std::vector<std::string> template_ids;
    int version = 1;
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

// Planned launch/origin reference is distinct from the first route waypoint and recovery point.
// A point-only anchor translates geographic geometry; it does not define local-frame rotation.
struct MissionOrigin {
    double longitude_deg = 0.0;
    double latitude_deg = 0.0;
    std::string coordinate_reference_system = "EPSG:4326";
    std::string origin_kind = "PLANNED_TAKEOFF";
    std::optional<double> horizontal_accuracy_m;
    std::optional<double> true_north_bearing_deg;
};

struct Mission {
    MissionIdentity identity;
    // AUTO and MANUAL share the same canonical model and safety pipeline.
    std::string mission_mode = "AUTO";
    std::optional<MissionOrigin> origin;
    std::vector<std::string> selected_template_ids;
    std::vector<MissionGeometryObject> geometry_objects;
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
