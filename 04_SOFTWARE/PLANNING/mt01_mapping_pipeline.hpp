#pragma once

#include "coverage_path_planner.hpp"
#include "mt01_kinematic_geometry_builder.hpp"
#include "mt01_route_candidate_builder.hpp"
#include "notam_geometry_adapter.hpp"
#include "model/flight_safety_constraints.hpp"
#include "model/mission_model.hpp"

#include <string>
#include <vector>

namespace bluesky::planning {

enum class Mt01PipelineStatus {
    NeedsInput,
    CoverageCandidateAvailable
};

struct Mt01MappingRequest {
    Mission mission;
    CoverageCameraModel camera;
    std::vector<std::vector<GeoPoint>> exclusion_areas; // Additional operator-defined exclusions.
    NotamSnapshot notam_snapshot;
    std::int64_t planned_mission_start_epoch_s{0};
    std::int64_t planned_mission_end_epoch_s{0};
    double notam_safety_buffer_m{0.0};
    double minimum_altitude_agl_m{0.0};
    double maximum_altitude_agl_m{0.0};
    double cruise_speed_mps{0.0};
    double line_angle_deg{0.0};
    VehicleMotionModel vehicle_motion_model;
    // Explicit, independently validated envelope; never inferred from motion estimates.
    FlightSafetyEnvelope flight_safety_envelope;
    Mt01RouteCandidateMetadata route_candidate_metadata;
};

struct Mt01MappingPipelineResult {
    Mt01PipelineStatus status{Mt01PipelineStatus::NeedsInput};
    bool coverage_candidate_available{false};
    bool flight_release_eligible{false};
    NotamGeometryAdapterResult notam_geometry;
    CoveragePlanResult coverage;
    bool kinematic_geometry_available{false};
    Mt01KinematicGeometryResult kinematic_geometry;
    bool segment_dynamics_validated{false};
    FlightSafetyValidationResult flight_safety_validation;
    bool route_candidate_available{false};
    Mt01RouteCandidateResult route_candidate;
    std::vector<std::string> findings;
};

// Joins the canonical mission aggregate to the existing 2D coverage generator.
// This stage produces a geometry candidate only. It never declares flight readiness.
class Mt01MappingPipeline {
public:
    static Mt01MappingPipelineResult calculate(const Mt01MappingRequest& request);
};

} // namespace bluesky::planning
