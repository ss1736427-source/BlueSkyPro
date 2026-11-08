#include "weather_coverage_request.hpp"

#include <cassert>
#include <cmath>

using namespace bluesky::planning;
using namespace bluesky::weather;

namespace {
Mission fixtureMission()
{
    Mission mission;
    mission.identity.mission_id = "MISSION-WX-01";
    mission.identity.mission_version = "3";
    mission.selected_solution.calculation_input_version = "INPUT-7";
    mission.origin = MissionOrigin{37.60, 55.70, "EPSG:4326", "PLANNED_TAKEOFF", 5.0, 0.0};
    mission.geometry_objects.push_back({
        "AOI-01", "POLYGON_AOI", "EPSG:4326",
        {{37.60,55.70,std::nullopt,std::nullopt},
         {37.61,55.70,std::nullopt,std::nullopt},
         {37.61,55.71,std::nullopt,std::nullopt},
         {37.60,55.71,std::nullopt,std::nullopt}},
        {"TEMPLATE-SURVEY"}, 2});
    return mission;
}

Route fixtureRoute()
{
    Route route;
    route.lineage.route_id = "ROUTE-01";
    route.lineage.route_version = "4";
    route.lineage.mission_id = "MISSION-WX-01";
    route.lineage.mission_version = "3";
    route.lineage.calculation_input_version = "INPUT-7";
    route.waypoints = {
        {"WP-1", {55.70, 37.60}, 50.0, true, AltitudeReference::AGL},
        {"WP-2", {55.72, 37.62}, 80.0, true, AltitudeReference::AGL}
    };
    return route;
}

WeatherCoverageRequestOptions fixtureOptions()
{
    WeatherCoverageRequestOptions options;
    options.spatial_buffer_m = 1000.0;
    options.time_window = {"2026-11-08T06:00:00Z", "2026-11-08T09:00:00Z"};
    options.required_area_altitudes_agl_m = {30.0, 50.0};
    return options;
}
}

int main()
{
    const auto mission = fixtureMission();
    const auto route = fixtureRoute();
    const auto options = fixtureOptions();

    const auto result = WeatherCoverageRequestBuilder::build(mission, route, options);
    assert(result.valid);
    assert(result.errors.empty());
    assert(result.request.mission_id == "MISSION-WX-01");
    assert(result.request.mission_version == "3");
    assert(result.request.route_id == "ROUTE-01");
    assert(result.request.route_version == "4");
    assert(result.request.components.size() == 3); // AOI + route + planned origin
    assert(result.request.bounds.south_deg < 55.70);
    assert(result.request.bounds.north_deg > 55.72);
    assert(result.request.bounds.west_deg < 37.60);
    assert(result.request.bounds.east_deg > 37.62);
    assert(result.request.required_altitudes_agl_m.size() == 3);
    assert(result.request.required_altitudes_agl_m[0] == 30.0);
    assert(result.request.required_altitudes_agl_m[1] == 50.0);
    assert(result.request.required_altitudes_agl_m[2] == 80.0);
    assert(result.request.provider_levels_agl_m.size() == 3);
    assert(result.request.provider_levels_agl_m[0] == 30);
    assert(result.request.provider_levels_agl_m[1] == 50);
    assert(result.request.provider_levels_agl_m[2] == 80);

    auto mismatchedRoute = route;
    mismatchedRoute.lineage.mission_version = "2";
    const auto mismatch = WeatherCoverageRequestBuilder::build(
        mission, mismatchedRoute, options);
    assert(!mismatch.valid);
    assert(!mismatch.errors.empty());

    auto invalidTime = options;
    invalidTime.time_window.valid_from_utc = "2026-11-08T06:00:00+00:00";
    const auto timeFailure = WeatherCoverageRequestBuilder::build(
        mission, route, invalidTime);
    assert(!timeFailure.valid);

    auto invalidAltitudeRoute = route;
    invalidAltitudeRoute.waypoints.front().altitude_reference = AltitudeReference::AMSL;
    const auto altitudeFailure = WeatherCoverageRequestBuilder::build(
        mission, invalidAltitudeRoute, options);
    assert(!altitudeFailure.valid);

    auto invalidBuffer = options;
    invalidBuffer.spatial_buffer_m = 0.0;
    const auto bufferFailure = WeatherCoverageRequestBuilder::build(
        mission, route, invalidBuffer);
    assert(!bufferFailure.valid);

    auto missingAreaLevels = options;
    missingAreaLevels.required_area_altitudes_agl_m.clear();
    const auto areaLevelFailure = WeatherCoverageRequestBuilder::build(
        mission, route, missingAreaLevels);
    assert(!areaLevelFailure.valid);

    auto interpolatedOptions = options;
    interpolatedOptions.required_area_altitudes_agl_m = {65.0};
    const auto interpolated = WeatherCoverageRequestBuilder::build(
        mission, route, interpolatedOptions);
    assert(interpolated.valid);
    assert(interpolated.request.provider_levels_agl_m.size() == 2);
    assert(interpolated.request.provider_levels_agl_m[0] == 50);
    assert(interpolated.request.provider_levels_agl_m[1] == 80);

    auto largeGapRoute = route;
    largeGapRoute.waypoints.front().altitude_m = 150.0;
    const auto largeGapFailure = WeatherCoverageRequestBuilder::build(
        mission, largeGapRoute, options);
    assert(!largeGapFailure.valid);

    auto datelineRoute = route;
    datelineRoute.waypoints.front().position.longitude_deg = 179.9;
    datelineRoute.waypoints.back().position.longitude_deg = -179.9;
    const auto datelineFailure = WeatherCoverageRequestBuilder::build(
        mission, datelineRoute, options);
    assert(!datelineFailure.valid);

    auto oversizedMission = mission;
    oversizedMission.geometry_objects.front().coordinates = {
        {0.0, 0.0, std::nullopt, std::nullopt},
        {31.0, 0.0, std::nullopt, std::nullopt},
        {31.0, 1.0, std::nullopt, std::nullopt}
    };
    const auto oversizedFailure = WeatherCoverageRequestBuilder::build(
        oversizedMission, route, options);
    assert(!oversizedFailure.valid);

    return 0;
}
