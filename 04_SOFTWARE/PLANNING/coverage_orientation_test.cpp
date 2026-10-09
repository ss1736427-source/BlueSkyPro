#include "coverage_orientation.hpp"

#include <cassert>

using namespace bluesky::planning;

int main() {
    AcquisitionGeometryInput geometry_input;
    geometry_input.equipment_id = "TEST-CAMERA";
    geometry_input.equipment_version = "TEST-1";
    geometry_input.sensor_width_m = 0.0132;
    geometry_input.sensor_height_m = 0.0088;
    geometry_input.focal_length_m = 0.0088;
    geometry_input.image_width_px = 5280.0;
    geometry_input.image_height_px = 3520.0;
    geometry_input.camera_ground_distance_m = 100.0;
    geometry_input.frontal_overlap_ratio = 0.75;
    geometry_input.side_overlap_ratio = 0.60;
    geometry_input.ground_speed_mps = 10.0;
    const auto geometry =
        AcquisitionGeometryEngine::calculate(geometry_input, "TEST-CALC-1");
    assert(geometry.valid);

    CoverageOrientationInput input;
    input.aoi = {{51.0, 0.0}, {51.0, 0.001}, {51.001, 0.001}, {51.001, 0.0}};
    input.acquisition_geometry = geometry;
    input.start_angle_deg = 0.0;
    input.end_angle_deg = 90.0;
    input.angle_step_deg = 45.0;
    input.geometry_revision = "AOI-TEST-1";
    input.calculation_version = "TEST-ORIENTATION-1";

    const auto result = CoverageOrientationGenerator::generate(input);
    assert(result.valid);
    assert(result.candidates.size() == 3);
    assert(result.candidates[0].generation_index == 0);
    assert(result.candidates[1].generation_index == 1);
    assert(result.candidates[2].generation_index == 2);
    assert(result.candidates[0].orientation_deg == 0.0);
    assert(result.candidates[1].orientation_deg == 45.0);
    assert(result.candidates[2].orientation_deg == 90.0);
    assert(result.candidates[0].candidate_id == "MT01-ORIENT-0");
    assert(result.candidates[0].estimated_track_count > 0);
    assert(result.candidates[0].projected_width_m > 0.0);

    auto bad = input;
    bad.angle_step_deg = 0.0;
    assert(!CoverageOrientationGenerator::generate(bad).valid);

    auto reversed = input;
    reversed.start_angle_deg = 90.0;
    reversed.end_angle_deg = 0.0;
    assert(!CoverageOrientationGenerator::generate(reversed).valid);

    auto changed = input;
    changed.angle_step_deg = 30.0;
    const auto changed_result = CoverageOrientationGenerator::generate(changed);
    assert(changed_result.valid);
    assert(changed_result.candidates.size() == 4);
    assert(changed_result.dependency_identity != result.dependency_identity);

    return 0;
}
