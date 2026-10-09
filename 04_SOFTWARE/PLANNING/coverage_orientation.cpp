#include "coverage_orientation.hpp"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <limits>
#include <sstream>

namespace bluesky::planning {
namespace {
constexpr double kPi = 3.14159265358979323846;
constexpr double kMetersPerDegree = 111320.0;

struct XY { double x; double y; };

bool finite(double v) { return std::isfinite(v); }
bool positive(double v) { return finite(v) && v > 0.0; }

XY project(const GeoPoint& p, double reference_latitude) {
    return {p.longitude_deg * kMetersPerDegree *
                std::cos(reference_latitude * kPi / 180.0),
            p.latitude_deg * kMetersPerDegree};
}

XY rotate(XY p, double angle_rad) {
    const double c = std::cos(angle_rad), s = std::sin(angle_rad);
    return {c * p.x - s * p.y, s * p.x + c * p.y};
}

std::string canonicalDouble(double v) {
    std::ostringstream out;
    out << std::setprecision(std::numeric_limits<double>::max_digits10) << v;
    return out.str();
}

std::string dependency(const CoverageOrientationInput& i) {
    std::ostringstream out;
    out << i.geometry_revision << '|'
        << i.calculation_version << '|'
        << canonicalDouble(i.start_angle_deg) << '|'
        << canonicalDouble(i.end_angle_deg) << '|'
        << canonicalDouble(i.angle_step_deg) << '|'
        << canonicalDouble(i.acquisition_geometry.track_spacing_m) << '|';
    for (const auto& p : i.aoi)
        out << canonicalDouble(p.latitude_deg) << ',' << canonicalDouble(p.longitude_deg) << ';';
    return out.str();
}

CoverageOrientationResult invalid(
    const CoverageOrientationInput& i, const std::string& code) {
    CoverageOrientationResult r;
    r.failure_code = code;
    r.dependency_identity = dependency(i);
    return r;
}

} // namespace

CoverageOrientationResult CoverageOrientationGenerator::generate(
    const CoverageOrientationInput& input) {

    if (input.aoi.size() < 3)
        return invalid(input, "INVALID_AOI");

    for (const auto& p : input.aoi) {
        if (!finite(p.latitude_deg) || !finite(p.longitude_deg) ||
            p.latitude_deg < -90.0 || p.latitude_deg > 90.0 ||
            p.longitude_deg < -180.0 || p.longitude_deg > 180.0)
            return invalid(input, "INVALID_AOI_COORDINATE");
    }

    if (!input.acquisition_geometry.valid ||
        !positive(input.acquisition_geometry.track_spacing_m))
        return invalid(input, "INVALID_ACQUISITION_GEOMETRY");

    if (!finite(input.start_angle_deg) || !finite(input.end_angle_deg) ||
        !positive(input.angle_step_deg))
        return invalid(input, "INVALID_ORIENTATION_SEARCH");

    if (input.end_angle_deg < input.start_angle_deg)
        return invalid(input, "ORIENTATION_RANGE_REVERSED");

    const double reference_latitude = input.aoi.front().latitude_deg;
    std::vector<XY> points;
    points.reserve(input.aoi.size());
    for (const auto& p : input.aoi)
        points.push_back(project(p, reference_latitude));

    CoverageOrientationResult result;
    result.dependency_identity = dependency(input);

    std::size_t index = 0;
    const double span = input.end_angle_deg - input.start_angle_deg;
    const std::size_t count =
        static_cast<std::size_t>(std::floor(span / input.angle_step_deg + 1e-12)) + 1;

    for (std::size_t k = 0; k < count; ++k) {
        const double angle_deg =
            input.start_angle_deg + static_cast<double>(k) * input.angle_step_deg;
        if (angle_deg > input.end_angle_deg + 1e-12) break;

        const double angle_rad = angle_deg * kPi / 180.0;
        double min_x = std::numeric_limits<double>::infinity();
        double max_x = -std::numeric_limits<double>::infinity();

        for (const auto& point : points) {
            const auto rotated = rotate(point, -angle_rad);
            min_x = std::min(min_x, rotated.x);
            max_x = std::max(max_x, rotated.x);
        }

        const double width = max_x - min_x;
        if (!positive(width))
            continue;

        CoverageOrientationCandidate candidate;
        candidate.generation_index = index++;
        candidate.orientation_deg = angle_deg;
        candidate.projected_width_m = width;
        candidate.estimated_track_count =
            static_cast<std::size_t>(
                std::ceil(width / input.acquisition_geometry.track_spacing_m));
        candidate.candidate_id =
            "MT01-ORIENT-" + std::to_string(candidate.generation_index);
        result.candidates.push_back(std::move(candidate));
    }

    if (result.candidates.empty())
        return invalid(input, "NO_ORIENTATION_CANDIDATE");

    result.valid = true;
    return result;
}

} // namespace bluesky::planning
