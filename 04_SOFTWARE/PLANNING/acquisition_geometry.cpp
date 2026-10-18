#include "acquisition_geometry.hpp"

#include <cmath>
#include <iomanip>
#include <sstream>

namespace bluesky::planning {
namespace {

bool finite_positive(double value) {
    return std::isfinite(value) && value > 0.0;
}

bool finite_ratio(double value) {
    return std::isfinite(value) && value >= 0.0 && value < 1.0;
}

std::string dependency(
    const AcquisitionGeometryInput& i,
    const std::string& calculation_version) {
    std::ostringstream out;
    out << i.equipment_id << '|'
        << i.equipment_version << '|'
        << std::setprecision(17)
        << i.sensor_width_m << '|'
        << i.sensor_height_m << '|'
        << i.focal_length_m << '|'
        << i.image_width_px << '|'
        << i.image_height_px << '|'
        << i.camera_ground_distance_m << '|'
        << i.frontal_overlap_ratio << '|'
        << i.side_overlap_ratio << '|'
        << i.ground_speed_mps << '|'
        << calculation_version;
    return out.str();
}

AcquisitionGeometryResult invalid(
    const AcquisitionGeometryInput& input,
    const std::string& version,
    const std::string& code) {
    AcquisitionGeometryResult result;
    result.failure_code = code;
    result.dependency_identity = dependency(input, version);
    return result;
}

} // namespace

AcquisitionGeometryResult AcquisitionGeometryEngine::calculate(
    const AcquisitionGeometryInput& input,
    const std::string& calculation_version) {

    if (input.equipment_id.empty() || input.equipment_version.empty())
        return invalid(input, calculation_version, "MISSING_EQUIPMENT_IDENTITY");

    if (!finite_positive(input.sensor_width_m) ||
        !finite_positive(input.sensor_height_m) ||
        !finite_positive(input.focal_length_m) ||
        !finite_positive(input.image_width_px) ||
        !finite_positive(input.image_height_px))
        return invalid(input, calculation_version, "INVALID_SENSOR_GEOMETRY");

    if (!finite_positive(input.camera_ground_distance_m))
        return invalid(input, calculation_version, "INVALID_CAMERA_GROUND_DISTANCE");

    if (!finite_ratio(input.frontal_overlap_ratio) ||
        !finite_ratio(input.side_overlap_ratio))
        return invalid(input, calculation_version, "INVALID_OVERLAP_RATIO");

    if (!finite_positive(input.ground_speed_mps))
        return invalid(input, calculation_version, "INVALID_GROUND_SPEED");

    AcquisitionGeometryResult result;
    result.dependency_identity = dependency(input, calculation_version);
    result.camera_ground_distance_m = input.camera_ground_distance_m;

    // Pinhole-model approximation from the controlled MT-01 algorithm contract.
    result.gsd_width_m_per_px =
        input.camera_ground_distance_m * input.sensor_width_m /
        (input.focal_length_m * input.image_width_px);

    result.gsd_height_m_per_px =
        input.camera_ground_distance_m * input.sensor_height_m /
        (input.focal_length_m * input.image_height_px);

    result.footprint_width_m =
        result.gsd_width_m_per_px * input.image_width_px;

    result.footprint_height_m =
        result.gsd_height_m_per_px * input.image_height_px;

    result.track_spacing_m =
        result.footprint_width_m * (1.0 - input.side_overlap_ratio);

    result.image_spacing_m =
        result.footprint_height_m * (1.0 - input.frontal_overlap_ratio);

    if (!finite_positive(result.track_spacing_m) ||
        !finite_positive(result.image_spacing_m))
        return invalid(input, calculation_version, "NON_POSITIVE_ACQUISITION_SPACING");

    result.trigger_interval_s =
        result.image_spacing_m / input.ground_speed_mps;

    result.trigger_rate_hz =
        1.0 / result.trigger_interval_s;
    result.frontal_overlap_ratio = input.frontal_overlap_ratio;
    result.side_overlap_ratio = input.side_overlap_ratio;

    if (!finite_positive(result.trigger_interval_s) ||
        !finite_positive(result.trigger_rate_hz))
        return invalid(input, calculation_version, "INVALID_ACQUISITION_TIMING");

    result.valid = true;
    return result;
}

} // namespace bluesky::planning
