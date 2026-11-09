#pragma once
#include "model/route_model.hpp"
#include <cstddef>
#include <string>
#include <vector>
namespace bluesky::planning {
struct CoverageCameraModel {
 double focal_length_mm{0.0}; double pixel_pitch_um{0.0};
 unsigned image_width_px{0}; unsigned image_height_px{0};
 double target_gsd_cm_px{0.0}; double forward_overlap_percent{0.0}; double side_overlap_percent{0.0};
 // Reference optical geometry is not an individual camera calibration.
 std::string camera_configuration_id;
 std::string calibration_source_reference;
 bool intrinsic_calibration_verified{false};
};
struct CoverageMissionInput {
 std::string mission_id; std::string mission_version;
 std::vector<GeoPoint> area_of_interest;
 std::vector<std::vector<GeoPoint>> exclusion_areas;
 CoverageCameraModel camera;
 double minimum_altitude_agl_m{0.0}; double maximum_altitude_agl_m{0.0};
 double cruise_speed_mps{0.0}; double line_angle_deg{0.0};
};
struct CoverageMetrics {
 double target_gsd_cm_px{0.0}; double calculated_altitude_agl_m{0.0};
 double footprint_width_m{0.0}; double footprint_height_m{0.0};
 double line_spacing_m{0.0}; double forward_trigger_spacing_m{0.0};
 // Coverage legs exclude transitions, turns and ferry legs.
 double estimated_coverage_leg_length_m{0.0}; double estimated_coverage_leg_time_s{0.0};
 std::size_t coverage_line_count{0}; std::size_t coverage_segment_count{0};
 std::size_t exclusion_intersection_count{0};
};
struct CoverageCapturePoint {
 GeoPoint position;
 std::size_t coverage_segment_index{0};
 double distance_from_segment_start_m{0.0};
};
struct CoveragePlanResult {
 bool valid{false}; std::string status; std::vector<GeoPoint> waypoints;
 std::vector<CoverageCapturePoint> capture_points;
 CoverageMetrics metrics; std::vector<std::string> diagnostics;
 bool intrinsic_calibration_verified{false};
};
// 2D candidate generator; emits image trigger positions along straight coverage legs only.
// Assumes image_height_px is the along-track sensor dimension; no turns, transit, DEM or safety validation.
class CoveragePathPlanner { public: static CoveragePlanResult generate(const CoverageMissionInput& input); };
} // namespace bluesky::planning
