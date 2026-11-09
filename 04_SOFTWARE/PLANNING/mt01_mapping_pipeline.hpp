#pragma once

#include "coverage_path_planner.hpp"
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
    std::vector<std::vector<GeoPoint>> exclusion_areas;
    double minimum_altitude_agl_m{0.0};
    double maximum_altitude_agl_m{0.0};
    double cruise_speed_mps{0.0};
    double line_angle_deg{0.0};
};

struct Mt01MappingPipelineResult {
    Mt01PipelineStatus status{Mt01PipelineStatus::NeedsInput};
    bool coverage_candidate_available{false};
    bool flight_release_eligible{false};
    CoveragePlanResult coverage;
    std::vector<std::string> findings;
};

// Joins the canonical mission aggregate to the existing 2D coverage generator.
// This stage produces a geometry candidate only. It never declares flight readiness.
class Mt01MappingPipeline {
public:
    static Mt01MappingPipelineResult calculate(const Mt01MappingRequest& request);
};

} // namespace bluesky::planning
