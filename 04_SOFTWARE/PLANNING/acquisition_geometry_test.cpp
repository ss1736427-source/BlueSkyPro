#include "acquisition_geometry.hpp"

#include <cassert>
#include <cmath>

using namespace bluesky::planning;

int main() {
    AcquisitionGeometryInput input;
    input.equipment_id = "TEST-CAMERA";
    input.equipment_version = "TEST-1";
    input.sensor_width_m = 0.0132;
    input.sensor_height_m = 0.0088;
    input.focal_length_m = 0.0088;
    input.image_width_px = 5280.0;
    input.image_height_px = 3520.0;
    input.camera_ground_distance_m = 100.0;
    input.frontal_overlap_ratio = 0.75;
    input.side_overlap_ratio = 0.60;
    input.ground_speed_mps = 10.0;

    const auto result =
        AcquisitionGeometryEngine::calculate(input, "TEST-CALC-1");

    assert(result.valid);

    // Independent expected values derived from this existing unit-test fixture.
    // These are formula regression expectations, not controlled mission inputs.
    const auto near = [](double actual, double expected) {
        return std::abs(actual - expected) <= 1e-12;
    };
    assert(near(result.gsd_width_m_per_px, 150.0 / input.image_width_px));
    assert(near(result.gsd_height_m_per_px, 100.0 / input.image_height_px));
    assert(near(result.footprint_width_m, 150.0));
    assert(near(result.footprint_height_m, 100.0));
    assert(near(result.track_spacing_m, 60.0));
    assert(near(result.image_spacing_m, 25.0));
    assert(near(result.trigger_interval_s, 2.5));
    assert(near(result.trigger_rate_hz, 0.4));
    assert(!result.dependency_identity.empty());

    auto missing = input;
    missing.focal_length_m = 0.0;
    assert(!AcquisitionGeometryEngine::calculate(missing, "TEST-CALC-1").valid);

    auto invalid_overlap = input;
    invalid_overlap.side_overlap_ratio = 1.0;
    assert(!AcquisitionGeometryEngine::calculate(
        invalid_overlap, "TEST-CALC-1").valid);

    auto invalid_speed = input;
    invalid_speed.ground_speed_mps = 0.0;
    assert(!AcquisitionGeometryEngine::calculate(
        invalid_speed, "TEST-CALC-1").valid);

    auto changed = input;
    changed.camera_ground_distance_m = 120.0;
    const auto changed_result =
        AcquisitionGeometryEngine::calculate(changed, "TEST-CALC-1");
    assert(changed_result.valid);
    assert(changed_result.dependency_identity != result.dependency_identity);
    assert(changed_result.gsd_width_m_per_px > result.gsd_width_m_per_px);

    return 0;
}
