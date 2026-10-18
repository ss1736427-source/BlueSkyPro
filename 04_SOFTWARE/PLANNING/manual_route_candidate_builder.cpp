#include "manual_route_candidate_builder.hpp"

#include "route_constraint_validator.hpp"

#include <algorithm>
#include <cmath>
#include <sstream>

namespace bluesky::planning {
namespace {

constexpr double kEarthRadiusM = 6371008.8;
constexpr double kPi = 3.14159265358979323846;

bool finite(double value)
{
    return std::isfinite(value);
}

double radians(double degrees)
{
    return degrees * kPi / 180.0;
}

double distanceMeters(const GeoPoint& a, const GeoPoint& b)
{
    const double lat1 = radians(a.latitude_deg);
    const double lat2 = radians(b.latitude_deg);
    const double deltaLat = lat2 - lat1;
    const double deltaLon = radians(b.longitude_deg - a.longitude_deg);
    const double sinLat = std::sin(deltaLat / 2.0);
    const double sinLon = std::sin(deltaLon / 2.0);
    const double h = sinLat * sinLat
        + std::cos(lat1) * std::cos(lat2) * sinLon * sinLon;
    return 2.0 * kEarthRadiusM
        * std::asin(std::sqrt(std::clamp(h, 0.0, 1.0)));
}

bool isSupportedLineGeometry(const std::string& type)
{
    return type == "WAYPOINT_ROUTE"
        || type == "CORRIDOR"
        || type == "DELIVERY_ROUTE";
}

void requireNonEmpty(std::vector<std::string>& errors,
                     const std::string& value,
                     const char* error)
{
    if (value.empty())
        errors.emplace_back(error);
}

} // namespace

ManualRouteBuildResult buildManualRouteCandidate(
    const Mission& mission,
    const MissionGeometryObject& geometry,
    const ManualRouteBuildRequirements& requirements)
{
    ManualRouteBuildResult result;

    if (mission.mission_mode != "MANUAL")
        result.errors.emplace_back("MANUAL_MISSION_REQUIRED");
    requireNonEmpty(result.errors, mission.identity.mission_id, "MISSION_ID_REQUIRED");
    requireNonEmpty(result.errors, mission.identity.mission_version, "MISSION_VERSION_REQUIRED");
    requireNonEmpty(result.errors, geometry.geometry_id, "GEOMETRY_ID_REQUIRED");

    if (geometry.coordinate_reference_system != "EPSG:4326")
        result.errors.emplace_back("GEOMETRY_CRS_UNSUPPORTED");
    if (!isSupportedLineGeometry(geometry.geometry_type))
        result.errors.emplace_back("TASK_SPECIFIC_GEOMETRY_GENERATOR_REQUIRED:"
                                   + geometry.geometry_type);
    if (geometry.coordinates.size() < 2)
        result.errors.emplace_back("ROUTE_REQUIRES_AT_LEAST_TWO_VERTICES");

    if (geometry.template_ids.empty())
        result.errors.emplace_back("GEOMETRY_TEMPLATE_ASSOCIATION_REQUIRED");
    for (const auto& templateId : geometry.template_ids) {
        if (std::find(mission.selected_template_ids.begin(),
                      mission.selected_template_ids.end(), templateId)
            == mission.selected_template_ids.end()) {
            result.errors.emplace_back("GEOMETRY_TEMPLATE_NOT_SELECTED:" + templateId);
        }
    }

    requireNonEmpty(result.errors, requirements.generator_id, "ROUTE_GENERATOR_ID_REQUIRED");
    requireNonEmpty(result.errors, requirements.generator_version, "ROUTE_GENERATOR_VERSION_REQUIRED");
    requireNonEmpty(result.errors, requirements.calculation_input_version,
                    "CALCULATION_INPUT_VERSION_REQUIRED");
    requireNonEmpty(result.errors, requirements.terrain_snapshot_id, "TERRAIN_SNAPSHOT_REQUIRED");
    requireNonEmpty(result.errors, requirements.airspace_snapshot_id, "AIRSPACE_SNAPSHOT_REQUIRED");
    requireNonEmpty(result.errors, requirements.weather_snapshot_id, "WEATHER_SNAPSHOT_REQUIRED");
    requireNonEmpty(result.errors, requirements.wind_snapshot_id, "WIND_SNAPSHOT_REQUIRED");

    if (!finite(requirements.minimum_target_agl_m)
        || !finite(requirements.maximum_target_agl_m)
        || !finite(requirements.minimum_clearance_m)
        || requirements.minimum_target_agl_m <= 0.0
        || requirements.maximum_target_agl_m < requirements.minimum_target_agl_m
        || requirements.minimum_clearance_m < 0.0) {
        result.errors.emplace_back("EXPLICIT_VALID_TARGET_AGL_LIMITS_REQUIRED");
    }
    if (!finite(requirements.nominal_speed_mps) || requirements.nominal_speed_mps <= 0.0)
        result.errors.emplace_back("EXPLICIT_POSITIVE_NOMINAL_SPEED_REQUIRED");

    for (std::size_t i = 0; i < geometry.coordinates.size(); ++i) {
        const auto& vertex = geometry.coordinates[i];
        if (!finite(vertex.longitude_deg) || vertex.longitude_deg < -180.0
            || vertex.longitude_deg > 180.0 || !finite(vertex.latitude_deg)
            || vertex.latitude_deg < -90.0 || vertex.latitude_deg > 90.0) {
            result.errors.emplace_back("GEOMETRY_COORDINATE_INVALID:" + std::to_string(i));
            continue;
        }
        if (!vertex.altitude_m || !finite(*vertex.altitude_m))
            result.errors.emplace_back("EXPLICIT_TARGET_ALTITUDE_REQUIRED:" + std::to_string(i));
        if (!vertex.altitude_reference || *vertex.altitude_reference != "AGL")
            result.errors.emplace_back("EXPLICIT_TARGET_ALTITUDE_REFERENCE_AGL_REQUIRED:"
                                      + std::to_string(i));
        else if (vertex.altitude_m
                 && (*vertex.altitude_m < requirements.minimum_target_agl_m
                     || *vertex.altitude_m > requirements.maximum_target_agl_m))
            result.errors.emplace_back("TARGET_ALTITUDE_OUTSIDE_CONFIGURED_AGL_LIMITS:"
                                       + std::to_string(i));
    }

    if (!result.errors.empty()) {
        std::sort(result.errors.begin(), result.errors.end());
        result.errors.erase(std::unique(result.errors.begin(), result.errors.end()),
                            result.errors.end());
        return result;
    }

    Route route;
    route.lineage.route_id = mission.identity.mission_id + "-ROUTE-" + geometry.geometry_id;
    route.lineage.route_version = std::to_string(geometry.version);
    route.lineage.mission_id = mission.identity.mission_id;
    route.lineage.mission_version = mission.identity.mission_version;
    route.lineage.generator_id = requirements.generator_id;
    route.lineage.generator_version = requirements.generator_version;
    route.lineage.calculation_input_version = requirements.calculation_input_version;
    route.constraints.minimum_altitude_m = requirements.minimum_target_agl_m;
    route.constraints.maximum_altitude_m = requirements.maximum_target_agl_m;
    route.constraints.minimum_clearance_m = requirements.minimum_clearance_m;
    route.constraints.avoid_restricted_areas = true;
    route.environment.terrain_snapshot_id = requirements.terrain_snapshot_id;
    route.environment.airspace_snapshot_id = requirements.airspace_snapshot_id;
    route.environment.weather_snapshot_id = requirements.weather_snapshot_id;
    route.environment.wind_snapshot_id = requirements.wind_snapshot_id;

    route.waypoints.reserve(geometry.coordinates.size());
    for (std::size_t i = 0; i < geometry.coordinates.size(); ++i) {
        const auto& vertex = geometry.coordinates[i];
        RouteWaypoint waypoint;
        waypoint.waypoint_id = geometry.geometry_id + "-WP-" + std::to_string(i + 1);
        waypoint.position = {vertex.latitude_deg, vertex.longitude_deg};
        waypoint.altitude_m = *vertex.altitude_m;
        waypoint.altitude_reference = AltitudeReference::AGL;
        waypoint.mandatory = true;
        route.waypoints.push_back(std::move(waypoint));
    }

    route.segments.reserve(route.waypoints.size() - 1);
    for (std::size_t i = 0; i + 1 < route.waypoints.size(); ++i) {
        const auto& from = route.waypoints[i];
        const auto& to = route.waypoints[i + 1];
        const double distance = distanceMeters(from.position, to.position);
        if (!finite(distance) || distance <= 0.0) {
            result.errors.emplace_back("ROUTE_HAS_ZERO_OR_INVALID_SEGMENT:"
                                       + std::to_string(i));
            continue;
        }
        route.segments.push_back({
            geometry.geometry_id + "-SEG-" + std::to_string(i + 1),
            from.waypoint_id,
            to.waypoint_id,
            distance,
            requirements.nominal_speed_mps
        });
    }

    if (result.errors.empty()) {
        const auto validation = RouteConstraintValidator::validate(route);
        for (const auto& finding : validation.findings) {
            std::ostringstream error;
            error << "CANONICAL_ROUTE_VALIDATION_FAILED:"
                  << static_cast<int>(finding.code) << ':' << finding.object_id;
            result.errors.push_back(error.str());
        }
    }

    if (result.errors.empty()) {
        result.route = std::move(route);
        result.valid = true;
    } else {
        std::sort(result.errors.begin(), result.errors.end());
        result.errors.erase(std::unique(result.errors.begin(), result.errors.end()),
                            result.errors.end());
    }
    return result;
}

} // namespace bluesky::planning
