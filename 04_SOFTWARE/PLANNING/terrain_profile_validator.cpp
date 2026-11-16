#include "terrain_profile_validator.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

namespace bluesky::planning {
namespace {
constexpr double kEarthRadiusM = 6371008.8;
constexpr double kPi = 3.14159265358979323846;

double radians(double degrees) { return degrees * kPi / 180.0; }

double distanceM(const GeoPoint& a, const GeoPoint& b)
{
    const double lat1 = radians(a.latitude_deg);
    const double lat2 = radians(b.latitude_deg);
    const double dLat = lat2 - lat1;
    const double dLon = radians(b.longitude_deg - a.longitude_deg);
    const double sLat = std::sin(dLat * 0.5);
    const double sLon = std::sin(dLon * 0.5);
    const double h = sLat * sLat + std::cos(lat1) * std::cos(lat2) * sLon * sLon;
    return 2.0 * kEarthRadiusM * std::asin(std::sqrt(std::clamp(h, 0.0, 1.0)));
}

GeoPoint interpolateGreatCircle(const GeoPoint& a, const GeoPoint& b, double t)
{
    const double lat1 = radians(a.latitude_deg), lon1 = radians(a.longitude_deg);
    const double lat2 = radians(b.latitude_deg), lon2 = radians(b.longitude_deg);
    const double x1 = std::cos(lat1) * std::cos(lon1);
    const double y1 = std::cos(lat1) * std::sin(lon1);
    const double z1 = std::sin(lat1);
    const double x2 = std::cos(lat2) * std::cos(lon2);
    const double y2 = std::cos(lat2) * std::sin(lon2);
    const double z2 = std::sin(lat2);
    const double dot = std::clamp(x1*x2 + y1*y2 + z1*z2, -1.0, 1.0);
    const double omega = std::acos(dot);
    if (omega < 1e-12)
        return {a.latitude_deg + (b.latitude_deg-a.latitude_deg)*t,
                a.longitude_deg + (b.longitude_deg-a.longitude_deg)*t};
    const double sinOmega = std::sin(omega);
    const double wa = std::sin((1.0-t)*omega) / sinOmega;
    const double wb = std::sin(t*omega) / sinOmega;
    const double x = wa*x1 + wb*x2;
    const double y = wa*y1 + wb*y2;
    const double z = wa*z1 + wb*z2;
    return {std::atan2(z, std::hypot(x,y))*180.0/kPi,
            std::atan2(y,x)*180.0/kPi};
}

bool interpolateElevation(const TerrainDemGrid& dem, const GeoPoint& point, double& elevation)
{
    double longitudeDelta = point.longitude_deg - dem.origin_longitude_deg;
    while (longitudeDelta > 180.0) longitudeDelta -= 360.0;
    while (longitudeDelta < -180.0) longitudeDelta += 360.0;
    const double x = longitudeDelta / dem.longitude_step_deg;
    const double y = (point.latitude_deg - dem.origin_latitude_deg) / dem.latitude_step_deg;
    if (!std::isfinite(x) || !std::isfinite(y) || x < 0.0 || y < 0.0
        || x > static_cast<double>(dem.columns-1)
        || y > static_cast<double>(dem.rows-1))
        return false;

    const auto x0 = static_cast<std::size_t>(std::floor(x));
    const auto y0 = static_cast<std::size_t>(std::floor(y));
    const auto x1 = std::min(x0+1, dem.columns-1);
    const auto y1 = std::min(y0+1, dem.rows-1);
    const double tx = x - static_cast<double>(x0);
    const double ty = y - static_cast<double>(y0);
    const auto at = [&](std::size_t row, std::size_t column) {
        return dem.elevation_m[row*dem.columns+column];
    };
    const double weights[4] = {
        (1.0-tx)*(1.0-ty), tx*(1.0-ty), (1.0-tx)*ty, tx*ty};
    const double values[4] = {
        at(y0,x0), at(y0,x1), at(y1,x0), at(y1,x1)};
    elevation = 0.0;
    for (std::size_t i=0;i<4;++i) {
        if (weights[i] <= 1e-12)
            continue;
        if (!std::isfinite(values[i]))
            return false;
        elevation += weights[i]*values[i];
    }
    return std::isfinite(elevation);
}

void fail(TerrainProfileAssessment& result, const char* code)
{
    result.findings.emplace_back(code);
}
} // namespace

TerrainProfileAssessment TerrainProfileValidator::evaluate(
    const Route& route, const TerrainDemGrid& dem,
    const TerrainProfileValidationOptions& options)
{
    TerrainProfileAssessment result;
    auto& snapshot = result.snapshot;
    auto& validation = result.validation;
    snapshot.snapshot_id = dem.snapshot_id;
    snapshot.source_id = dem.source_id;
    snapshot.source_version = dem.source_version;
    snapshot.dataset_id = dem.dataset_id;
    snapshot.dataset_version = dem.dataset_version;
    snapshot.horizontal_crs = dem.horizontal_crs;
    snapshot.vertical_datum = dem.vertical_datum;
    snapshot.horizontal_resolution_m = dem.horizontal_resolution_m;
    snapshot.vertical_accuracy_m = dem.vertical_accuracy_m;

    validation.validator_id = "terrain-profile-dem-grid-validator";
    validation.validator_version = "1";
    validation.snapshot_id = dem.snapshot_id;
    validation.route_id = route.lineage.route_id;
    validation.route_version = route.lineage.route_version;
    validation.calculation_input_version = route.lineage.calculation_input_version;

    const bool metadataValid = !dem.snapshot_id.empty() && !dem.source_id.empty()
        && !dem.source_version.empty() && !dem.dataset_id.empty()
        && !dem.dataset_version.empty() && dem.horizontal_crs == "EPSG:4326"
        && !dem.vertical_datum.empty() && dem.vertical_datum != "UNKNOWN"
        && dem.vertical_datum != "UNSPECIFIED"
        && std::isfinite(dem.horizontal_resolution_m) && dem.horizontal_resolution_m > 0.0
        && std::isfinite(dem.vertical_accuracy_m) && dem.vertical_accuracy_m > 0.0
        && std::isfinite(dem.origin_latitude_deg) && std::abs(dem.origin_latitude_deg) <= 90.0
        && std::isfinite(dem.origin_longitude_deg) && std::abs(dem.origin_longitude_deg) <= 180.0
        && std::isfinite(dem.latitude_step_deg) && dem.latitude_step_deg > 0.0
        && std::isfinite(dem.longitude_step_deg) && dem.longitude_step_deg > 0.0
        && dem.rows >= 2 && dem.columns >= 2
        && dem.rows <= std::numeric_limits<std::size_t>::max()/dem.columns
        && dem.elevation_m.size() == dem.rows*dem.columns;
    if (!metadataValid) fail(result, "DEM_METADATA_OR_GRID_INVALID");
    if (route.environment.terrain_snapshot_id != dem.snapshot_id)
        fail(result, "DEM_SNAPSHOT_NOT_BOUND_TO_ROUTE");
    if (!route.trajectory_geometry_complete || route.waypoints.size() < 2
        || route.segments.size()+1 != route.waypoints.size())
        fail(result, "COMPLETE_ROUTE_TRAJECTORY_REQUIRED");
    if (route.lineage.mission_id.empty() || route.lineage.mission_version.empty()
        || route.lineage.route_id.empty() || route.lineage.route_version.empty()
        || route.lineage.calculation_input_version.empty())
        fail(result, "ROUTE_LINEAGE_REQUIRED");
    if (!std::isfinite(options.maximum_sample_gap_m) || options.maximum_sample_gap_m < 0.0
        || !std::isfinite(options.additional_clearance_margin_m)
        || options.additional_clearance_margin_m < 0.0
        || options.maximum_profile_samples < 2)
        fail(result, "TERRAIN_VALIDATION_OPTIONS_INVALID");
    if (!metadataValid || !result.findings.empty())
        return result;

    const GeoPoint gridSouthWest{dem.origin_latitude_deg,dem.origin_longitude_deg};
    const GeoPoint gridSouthEast{dem.origin_latitude_deg,
        dem.origin_longitude_deg+dem.longitude_step_deg};
    const GeoPoint gridNorthWest{dem.origin_latitude_deg+dem.latitude_step_deg,
        dem.origin_longitude_deg};
    const double pixelEastM=distanceM(gridSouthWest,gridSouthEast);
    const double pixelNorthM=distanceM(gridSouthWest,gridNorthWest);
    const double nominalPixelM=std::sqrt(pixelEastM*pixelNorthM);
    if (!std::isfinite(nominalPixelM) || nominalPixelM <= 0.0
        || std::abs(nominalPixelM-dem.horizontal_resolution_m)
            > std::max(5.0,dem.horizontal_resolution_m*0.35)) {
        fail(result, "DEM_GRID_SPACING_DOES_NOT_MATCH_DECLARED_RESOLUTION");
        return result;
    }
    const double sampleGap = options.maximum_sample_gap_m > 0.0
        ? std::min(options.maximum_sample_gap_m, dem.horizontal_resolution_m*0.5)
        : dem.horizontal_resolution_m*0.5;
    if (!std::isfinite(sampleGap) || sampleGap <= 0.0) {
        fail(result, "TERRAIN_SAMPLE_GAP_INVALID");
        return result;
    }

    std::vector<double> waypointGround;
    waypointGround.reserve(route.waypoints.size());
    for (const auto& waypoint : route.waypoints) {
        if (waypoint.altitude_reference != AltitudeReference::AGL
            || !std::isfinite(waypoint.altitude_m) || waypoint.altitude_m < 0.0) {
            fail(result, "ROUTE_WAYPOINT_REQUIRES_VALID_AGL_HEIGHT");
            return result;
        }
        double ground = 0.0;
        if (!interpolateElevation(dem, waypoint.position, ground)) {
            fail(result, "DEM_WAYPOINT_COVERAGE_OR_NODATA_GAP");
            return result;
        }
        waypointGround.push_back(ground);
    }

    double minimumClearance = std::numeric_limits<double>::infinity();
    double maximumGap = 0.0;
    std::size_t sampleCount = 0;
    bool completeDemCoverage = true;
    for (std::size_t segmentIndex=0; segmentIndex<route.segments.size(); ++segmentIndex) {
        const auto& segment = route.segments[segmentIndex];
        const auto& from = route.waypoints[segmentIndex];
        const auto& to = route.waypoints[segmentIndex+1];
        if (segment.from_waypoint_id != from.waypoint_id
            || segment.to_waypoint_id != to.waypoint_id
            || !std::isfinite(segment.distance_m) || segment.distance_m <= 0.0) {
            fail(result, "ROUTE_SEGMENT_ORDER_OR_DISTANCE_INVALID");
            return result;
        }
        const double length = distanceM(from.position, to.position);
        if (!std::isfinite(length) || length <= 0.0
            || std::abs(length-segment.distance_m) > std::max(5.0, length*0.02)) {
            fail(result, "ROUTE_SEGMENT_GEOMETRY_DOES_NOT_MATCH_WAYPOINTS");
            return result;
        }
        const double requiredIntervals=std::ceil(length/sampleGap);
        if (!std::isfinite(requiredIntervals) || requiredIntervals < 1.0
            || requiredIntervals > static_cast<double>(options.maximum_profile_samples)) {
            fail(result, "TERRAIN_PROFILE_SAMPLE_LIMIT_EXCEEDED");
            return result;
        }
        const std::size_t intervals = static_cast<std::size_t>(requiredIntervals);
        const std::size_t additionalSamples = intervals + (segmentIndex==0 ? 1U : 0U);
        if (additionalSamples > options.maximum_profile_samples
            || sampleCount > options.maximum_profile_samples-additionalSamples) {
            fail(result, "TERRAIN_PROFILE_SAMPLE_LIMIT_EXCEEDED");
            return result;
        }
        const double actualGap = length/static_cast<double>(intervals);
        maximumGap = std::max(maximumGap, actualGap);
        for (std::size_t i=(segmentIndex==0 ? 0 : 1); i<=intervals; ++i) {
            const double t = static_cast<double>(i)/static_cast<double>(intervals);
            const GeoPoint position = interpolateGreatCircle(from.position,to.position,t);
            double ground = 0.0;
            if (!interpolateElevation(dem,position,ground)) {
                completeDemCoverage = false;
                fail(result, "DEM_ROUTE_COVERAGE_OR_NODATA_GAP");
                return result;
            }
            // AGL at each waypoint is converted to the DEM vertical datum at
            // that endpoint, then the absolute trajectory height is interpolated.
            const double fromAmsl = waypointGround[segmentIndex] + from.altitude_m;
            const double toAmsl = waypointGround[segmentIndex+1] + to.altitude_m;
            const double trajectoryAmsl = fromAmsl + (toAmsl-fromAmsl)*t;
            const double conservativeClearance = trajectoryAmsl - ground
                - dem.vertical_accuracy_m - options.additional_clearance_margin_m;
            if (!std::isfinite(conservativeClearance)) {
                fail(result, "TERRAIN_CLEARANCE_NON_FINITE");
                return result;
            }
            minimumClearance = std::min(minimumClearance,conservativeClearance);
            ++sampleCount;
        }
    }

    validation.profile_sample_count = sampleCount;
    validation.maximum_sample_gap_m = maximumGap;
    validation.minimum_observed_clearance_m = minimumClearance;
    validation.complete_trajectory_coverage = sampleCount >= 2;
    validation.vertical_datum_normalized = true;
    validation.findings_empty = result.findings.empty();
    validation.passed = validation.findings_empty
        && validation.complete_trajectory_coverage
        && std::isfinite(route.constraints.minimum_clearance_m)
        && route.constraints.minimum_clearance_m >= 0.0
        && minimumClearance >= route.constraints.minimum_clearance_m;
    if (!validation.passed && validation.findings.empty())
        fail(result, "MINIMUM_TERRAIN_CLEARANCE_NOT_MET");
    validation.findings_empty = result.findings.empty();
    validation.passed = validation.passed && validation.findings_empty;
    snapshot.coverage_complete = completeDemCoverage && sampleCount >= 2;
    return result;
}

} // namespace bluesky::planning
