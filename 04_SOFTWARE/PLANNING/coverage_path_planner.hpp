#pragma once

#include "model/route_model.hpp"
#include <string>
#include <vector>

namespace bluesky::planning {

struct CoverageCameraModel {
    double focal_length_mm{0.0};
    double pixel_pitch_um{0.0};
    unsigned image_width_px{0};
    unsigned image_height_px{0};
    double target_gsd_cm_px{0.0};
    double forward_overlap_percent{0.0};
    double side_overlap_percent{0.0};
};

struct CoverageMissionInput {
    std::string mission_id;
    std::string mission_version;
    std::vector<GeoPoint> area_of_interest;
    std::vector<std::vector<GeoPoint>> exclusion_areas;
    CoverageCameraModel camera;
    double minimum_altitude_agl_m{0.0};
    double maximum_altitude_agl_m{0.0};
    double cruise_speed_mps{0.0};
    double line_angle_deg{0.0};
};

struct CoverageMetrics {
    double target_gsd_cm_px{0.0};
    double calculated_altitude_agl_m{0.0};
    double footprint_width_m{0.0};
    double footprint_height_m{0.0};
    double line_spacing_m{0.0};
    double estimated_track_length_m{0.0};
    double estimated_flight_time_s{0.0};
    std::size_t coverage_line_count{0};
    std::size_t exclusion_gap_count{0};
};

struct CoveragePlanResult {
    bool valid{false};
    std::string status;
    std::vector<GeoPoint> waypoints;
    CoverageMetrics metrics;
    std::vector<std::string> diagnostics;
};

// Deterministic 2D sweep-line baseline. Terrain following, turn dynamics,
// obstacle routing and independent airspace/energy validation are separate stages.
class CoveragePathPlanner {
public:
    static CoveragePlanResult generate(const CoverageMissionInput& input);
};

} // namespace bluesky::planning
