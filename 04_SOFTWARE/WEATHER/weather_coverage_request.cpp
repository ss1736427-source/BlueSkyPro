#include "weather_coverage_request.hpp"

#include <algorithm>
#include <cmath>
#include <set>
#include <tuple>

namespace bluesky::weather {
namespace {

constexpr double kMetersPerDegreeLatitude = 111320.0;
constexpr double kPi = 3.14159265358979323846;

bool finite(double value)
{
    return std::isfinite(value);
}

bool validUtcTimestamp(const std::string& value)
{
    if (value.size() != 20 || value[4] != '-' || value[7] != '-'
        || value[10] != 'T' || value[13] != ':' || value[16] != ':'
        || value[19] != 'Z')
        return false;

    for (std::size_t i = 0; i < value.size(); ++i) {
        if (i == 4 || i == 7 || i == 10 || i == 13 || i == 16 || i == 19)
            continue;
        if (value[i] < '0' || value[i] > '9')
            return false;
    }

    const int year = std::stoi(value.substr(0, 4));
    const int month = std::stoi(value.substr(5, 2));
    const int day = std::stoi(value.substr(8, 2));
    const int hour = std::stoi(value.substr(11, 2));
    const int minute = std::stoi(value.substr(14, 2));
    const int second = std::stoi(value.substr(17, 2));
    if (year < 1970 || month < 1 || month > 12
        || hour > 23 || minute > 59 || second > 59)
        return false;

    static constexpr int daysPerMonth[] = {
        31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
    };
    const bool leapYear = (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
    const int maximumDay = daysPerMonth[month - 1] + (month == 2 && leapYear ? 1 : 0);
    return day >= 1 && day <= maximumDay;
}

bool validCoordinate(double latitude, double longitude)
{
    return finite(latitude) && finite(longitude)
        && latitude >= -90.0 && latitude <= 90.0
        && longitude >= -180.0 && longitude <= 180.0;
}

bool isWeatherGeometry(const std::string& type)
{
    return type == "POLYGON_AOI"
        || type == "COVERAGE_GRID"
        || type == "SEARCH_AREA"
        || type == "CORRIDOR"
        || type == "ORBIT"
        || type == "WAYPOINT_ROUTE"
        || type == "DELIVERY_ROUTE"
        || type == "INSPECTION_POINTS"
        || type == "OBSERVATION_POINTS";
}

std::size_t minimumWeatherGeometryVertices(const std::string& type)
{
    if (type == "INSPECTION_POINTS" || type == "OBSERVATION_POINTS")
        return 1;
    if (type == "CORRIDOR" || type == "WAYPOINT_ROUTE" || type == "DELIVERY_ROUTE")
        return 2;
    return 3;
}

void includeCoordinate(double latitude, double longitude,
                       double& south, double& west,
                       double& north, double& east,
                       bool& hasCoordinate)
{
    if (!hasCoordinate) {
        south = north = latitude;
        west = east = longitude;
        hasCoordinate = true;
        return;
    }
    south = std::min(south, latitude);
    west = std::min(west, longitude);
    north = std::max(north, latitude);
    east = std::max(east, longitude);
}

} // namespace

WeatherCoverageRequestBuildResult WeatherCoverageRequestBuilder::build(
    const bluesky::planning::Mission& mission,
    const bluesky::planning::Route& route,
    const WeatherCoverageRequestOptions& options)
{
    WeatherCoverageRequestBuildResult result;
    auto& errors = result.errors;

    if (mission.identity.mission_id.empty())
        errors.emplace_back("MISSION_ID_REQUIRED");
    if (mission.identity.mission_version.empty())
        errors.emplace_back("MISSION_VERSION_REQUIRED");
    if (route.lineage.route_id.empty())
        errors.emplace_back("ROUTE_ID_REQUIRED");
    if (route.lineage.route_version.empty())
        errors.emplace_back("ROUTE_VERSION_REQUIRED");
    if (route.lineage.mission_id != mission.identity.mission_id
        || route.lineage.mission_version != mission.identity.mission_version)
        errors.emplace_back("ROUTE_MISSION_LINEAGE_MISMATCH");
    if (!route.lineage.calculation_input_version.empty()
        && !mission.selected_solution.calculation_input_version.empty()
        && route.lineage.calculation_input_version
            != mission.selected_solution.calculation_input_version)
        errors.emplace_back("ROUTE_CALCULATION_INPUT_VERSION_MISMATCH");

    if (!finite(options.spatial_buffer_m) || options.spatial_buffer_m <= 0.0)
        errors.emplace_back("POSITIVE_SPATIAL_BUFFER_REQUIRED");
    if (!validUtcTimestamp(options.time_window.valid_from_utc)
        || !validUtcTimestamp(options.time_window.valid_to_utc)
        || options.time_window.valid_to_utc < options.time_window.valid_from_utc)
        errors.emplace_back("VALID_UTC_TIME_WINDOW_REQUIRED");

    if (route.waypoints.size() < 2)
        errors.emplace_back("ROUTE_REQUIRES_AT_LEAST_TWO_WAYPOINTS");

    WeatherCoverageRequest request;
    request.mission_id = mission.identity.mission_id;
    request.mission_version = mission.identity.mission_version;
    request.route_id = route.lineage.route_id;
    request.route_version = route.lineage.route_version;
    request.calculation_input_version = route.lineage.calculation_input_version;
    request.spatial_buffer_m = options.spatial_buffer_m;
    request.time_window = options.time_window;
    request.required_altitudes_agl_m = options.required_area_altitudes_agl_m;

    std::set<double> altitudeLevels;
    for (double altitude : options.required_area_altitudes_agl_m) {
        if (!finite(altitude) || altitude <= 0.0)
            errors.emplace_back("AREA_ALTITUDE_MUST_BE_POSITIVE_AGL");
        else
            altitudeLevels.insert(altitude);
    }

    double south = 0.0, west = 0.0, north = 0.0, east = 0.0;
    bool hasCoordinate = false;
    bool hasMissionGeometry = false;

    for (const auto& geometry : mission.geometry_objects) {
        if (!isWeatherGeometry(geometry.geometry_type))
            continue;
        hasMissionGeometry = true;
        if (geometry.geometry_id.empty() || geometry.version < 1) {
            errors.emplace_back("MISSION_GEOMETRY_ID_OR_VERSION_INVALID");
            continue;
        }
        if (geometry.coordinate_reference_system != "EPSG:4326") {
            errors.emplace_back("MISSION_GEOMETRY_CRS_UNSUPPORTED:" + geometry.geometry_id);
            continue;
        }
        if (geometry.coordinates.size() < minimumWeatherGeometryVertices(geometry.geometry_type)) {
            errors.emplace_back("MISSION_GEOMETRY_HAS_TOO_FEW_VERTICES:" + geometry.geometry_id);
            continue;
        }

        CoverageComponent component;
        component.kind = CoverageComponentKind::MissionGeometry;
        component.source_id = geometry.geometry_id;
        component.source_version = std::to_string(geometry.version);
        component.geometry_type = geometry.geometry_type;

        bool componentValid = true;
        for (const auto& vertex : geometry.coordinates) {
            if (!validCoordinate(vertex.latitude_deg, vertex.longitude_deg)) {
                errors.emplace_back("MISSION_GEOMETRY_COORDINATE_INVALID:" + geometry.geometry_id);
                componentValid = false;
                continue;
            }
            component.coordinates.push_back({vertex.latitude_deg, vertex.longitude_deg});
            includeCoordinate(vertex.latitude_deg, vertex.longitude_deg,
                              south, west, north, east, hasCoordinate);
        }
        if (componentValid)
            request.components.push_back(std::move(component));
    }

    CoverageComponent routeComponent;
    routeComponent.kind = CoverageComponentKind::Route;
    routeComponent.source_id = route.lineage.route_id;
    routeComponent.source_version = route.lineage.route_version;
    routeComponent.geometry_type = "ORDERED_ROUTE";
    bool routeValid = route.waypoints.size() >= 2;
    for (const auto& waypoint : route.waypoints) {
        if (!validCoordinate(waypoint.position.latitude_deg,
                             waypoint.position.longitude_deg)) {
            errors.emplace_back("ROUTE_COORDINATE_INVALID:" + waypoint.waypoint_id);
            routeValid = false;
            continue;
        }
        if (waypoint.altitude_reference != bluesky::planning::AltitudeReference::AGL
            || !finite(waypoint.altitude_m) || waypoint.altitude_m <= 0.0) {
            errors.emplace_back("ROUTE_WAYPOINT_REQUIRES_POSITIVE_AGL:" + waypoint.waypoint_id);
            routeValid = false;
        } else {
            altitudeLevels.insert(waypoint.altitude_m);
        }
        routeComponent.coordinates.push_back({
            waypoint.position.latitude_deg, waypoint.position.longitude_deg});
        includeCoordinate(waypoint.position.latitude_deg,
                          waypoint.position.longitude_deg,
                          south, west, north, east, hasCoordinate);
    }
    if (routeValid)
        request.components.push_back(std::move(routeComponent));

    if (mission.origin) {
        const auto& origin = *mission.origin;
        if (origin.coordinate_reference_system != "EPSG:4326"
            || origin.origin_kind != "PLANNED_TAKEOFF"
            || !validCoordinate(origin.latitude_deg, origin.longitude_deg)) {
            errors.emplace_back("MISSION_ORIGIN_INVALID_OR_UNSUPPORTED");
        } else {
            CoverageComponent component;
            component.kind = CoverageComponentKind::PlannedOrigin;
            component.source_id = "MISSION_ORIGIN";
            component.source_version = mission.identity.mission_version;
            component.geometry_type = "POINT";
            component.coordinates.push_back({origin.latitude_deg, origin.longitude_deg});
            request.components.push_back(std::move(component));
            includeCoordinate(origin.latitude_deg, origin.longitude_deg,
                              south, west, north, east, hasCoordinate);
        }
    }

    if (!hasCoordinate)
        errors.emplace_back("NO_VALID_SPATIAL_COMPONENTS");
    if (west < -180.0 || east > 180.0 || east - west > 180.0)
        errors.emplace_back("ANTIMERIDIAN_OR_LONGITUDE_WRAP_UNSUPPORTED");

    if (hasCoordinate && finite(options.spatial_buffer_m)
        && options.spatial_buffer_m > 0.0
        && south >= -90.0 && north <= 90.0
        && west >= -180.0 && east <= 180.0 && east - west <= 180.0) {
        const double latitudeBuffer = options.spatial_buffer_m / kMetersPerDegreeLatitude;
        const double maxAbsLatitude = std::max(std::abs(south), std::abs(north));
        const double longitudeScale = std::cos(maxAbsLatitude * kPi / 180.0);
        if (longitudeScale < 1e-6) {
            errors.emplace_back("BUFFER_LONGITUDE_UNDEFINED_NEAR_POLE");
        } else {
            const double longitudeBuffer =
                options.spatial_buffer_m / (kMetersPerDegreeLatitude * longitudeScale);
            request.bounds.south_deg = south - latitudeBuffer;
            request.bounds.north_deg = north + latitudeBuffer;
            request.bounds.west_deg = west - longitudeBuffer;
            request.bounds.east_deg = east + longitudeBuffer;
            if (request.bounds.south_deg < -90.0 || request.bounds.north_deg > 90.0
                || request.bounds.west_deg < -180.0 || request.bounds.east_deg > 180.0) {
                errors.emplace_back("BUFFER_EXCEEDS_SUPPORTED_GEOGRAPHIC_BOUNDS");
            } else if (request.bounds.east_deg - request.bounds.west_deg > 30.0
                       || request.bounds.north_deg - request.bounds.south_deg > 30.0) {
                errors.emplace_back("REQUEST_EXCEEDS_NOAA_30_DEGREE_REGION_LIMIT");
            }
        }
    }

    if (hasMissionGeometry && options.required_area_altitudes_agl_m.empty())
        errors.emplace_back("MISSION_GEOMETRY_AGL_ALTITUDE_LEVELS_REQUIRED");

    request.required_altitudes_agl_m.assign(altitudeLevels.begin(), altitudeLevels.end());
    if (request.required_altitudes_agl_m.empty()) {
        errors.emplace_back("REQUIRED_AGL_ALTITUDE_LEVELS_MISSING");
    } else {
        const std::vector<int> supportedProviderLevels{2, 10, 20, 30, 40, 50, 80, 100, 1000, 4000};
        std::set<int> providerLevels;
        for (double sampleAltitude : request.required_altitudes_agl_m) {
            const auto upper = std::lower_bound(
                supportedProviderLevels.begin(), supportedProviderLevels.end(), sampleAltitude,
                [](int level, double altitude) { return static_cast<double>(level) < altitude; });
            if (upper != supportedProviderLevels.end()
                && std::abs(static_cast<double>(*upper) - sampleAltitude) < 1e-6) {
                providerLevels.insert(*upper);
                continue;
            }
            if (upper == supportedProviderLevels.begin()
                || upper == supportedProviderLevels.end()) {
                errors.emplace_back("NOAA_PROVIDER_LEVELS_DO_NOT_BRACKET_AGL:"
                                    + std::to_string(sampleAltitude));
                continue;
            }
            const int lowerLevel = *(upper - 1);
            const int upperLevel = *upper;
            if (upperLevel - lowerLevel > 100) {
                errors.emplace_back("NOAA_PROVIDER_VERTICAL_GAP_EXCEEDS_100M:"
                                    + std::to_string(sampleAltitude));
                continue;
            }
            providerLevels.insert(lowerLevel);
            providerLevels.insert(upperLevel);
        }
        request.provider_levels_agl_m.assign(providerLevels.begin(), providerLevels.end());
    }

    if (errors.empty()) {
        result.request = std::move(request);
        result.valid = true;
    }
    return result;
}


WeatherCoverageRequestBuildResult WeatherCoverageRequestBuilder::build(
    const bluesky::planning::Mission& mission,
    const std::vector<bluesky::planning::Route>& candidateRoutes,
    const WeatherCoverageRequestOptions& options)
{
    WeatherCoverageRequestBuildResult result;
    if (candidateRoutes.empty()) {
        result.errors.emplace_back("WEATHER_COVERAGE_REQUIRES_AT_LEAST_ONE_CANDIDATE_ROUTE");
        return result;
    }

    std::set<std::tuple<int, std::string, std::string, std::string>> componentIdentities;
    std::set<double> requiredAltitudes;
    std::set<int> providerLevels;
    std::set<std::pair<std::string, std::string>> routeIdentities;
    bool first = true;
    std::string commonCalculationInputVersion;

    for (const auto& route : candidateRoutes) {
        const auto candidate = build(mission, route, options);
        if (!candidate.valid) {
            for (const auto& error : candidate.errors) {
                result.errors.emplace_back(
                    "CANDIDATE_ROUTE_" + route.lineage.route_id + ":" + error);
            }
            continue;
        }

        const auto identity = std::make_pair(
            route.lineage.route_id, route.lineage.route_version);
        if (!routeIdentities.insert(identity).second) {
            result.errors.emplace_back("DUPLICATE_CANDIDATE_ROUTE_IDENTITY:"
                                       + route.lineage.route_id);
            continue;
        }
        if (first) {
            result.request = candidate.request;
            result.request.components.clear();
            commonCalculationInputVersion = candidate.request.calculation_input_version;
            first = false;
        } else {
            if (candidate.request.calculation_input_version
                != commonCalculationInputVersion) {
                result.errors.emplace_back(
                    "CANDIDATE_ROUTES_USE_DIFFERENT_CALCULATION_INPUT_VERSIONS");
            }
            result.request.bounds.south_deg = std::min(
                result.request.bounds.south_deg, candidate.request.bounds.south_deg);
            result.request.bounds.west_deg = std::min(
                result.request.bounds.west_deg, candidate.request.bounds.west_deg);
            result.request.bounds.north_deg = std::max(
                result.request.bounds.north_deg, candidate.request.bounds.north_deg);
            result.request.bounds.east_deg = std::max(
                result.request.bounds.east_deg, candidate.request.bounds.east_deg);
        }

        result.request.candidate_routes.push_back({
            route.lineage.route_id, route.lineage.route_version});

        for (const auto& component : candidate.request.components) {
            const auto key = std::make_tuple(
                static_cast<int>(component.kind), component.source_id,
                component.source_version, component.geometry_type);
            if (componentIdentities.insert(key).second)
                result.request.components.push_back(component);
        }
        requiredAltitudes.insert(
            candidate.request.required_altitudes_agl_m.begin(),
            candidate.request.required_altitudes_agl_m.end());
        providerLevels.insert(
            candidate.request.provider_levels_agl_m.begin(),
            candidate.request.provider_levels_agl_m.end());
    }

    if (!result.errors.empty() || first) {
        if (first && result.errors.empty())
            result.errors.emplace_back("NO_VALID_CANDIDATE_ROUTE_COVERAGE");
        return result;
    }

    result.request.calculation_input_version = commonCalculationInputVersion;
    result.request.required_altitudes_agl_m.assign(
        requiredAltitudes.begin(), requiredAltitudes.end());
    result.request.provider_levels_agl_m.assign(
        providerLevels.begin(), providerLevels.end());
    if (result.request.candidate_routes.size() == 1) {
        result.request.route_id = result.request.candidate_routes.front().route_id;
        result.request.route_version = result.request.candidate_routes.front().route_version;
    } else {
        result.request.route_id = "MULTI_CANDIDATE";
        result.request.route_version = "MULTI_CANDIDATE";
    }
    result.valid = true;
    return result;
}

} // namespace bluesky::weather
