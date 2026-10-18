#include "manual_route_candidate_builder.hpp"

#include <algorithm>
#include <cassert>
#include <string>

using namespace bluesky::planning;

namespace {

Mission missionFixture()
{
    Mission mission;
    mission.identity.mission_id = "MISSION-MANUAL-001";
    mission.identity.mission_version = "7";
    mission.mission_mode = "MANUAL";
    mission.selected_template_ids = {"MT-05"};
    return mission;
}

MissionGeometryObject corridorFixture()
{
    return {
        "GEO-CORRIDOR-01",
        "CORRIDOR",
        "EPSG:4326",
        {
            {37.6000, 55.7000, 40.0, std::string("AGL")},
            {37.6100, 55.7050, 45.0, std::string("AGL")},
            {37.6200, 55.7100, 40.0, std::string("AGL")}
        },
        {"MT-05"},
        2
    };
}

ManualRouteBuildRequirements requirementsFixture()
{
    return {
        "manual-line-route-generator",
        "1",
        "INPUT-7",
        "TERRAIN-42",
        "AIRSPACE-17",
        "WEATHER-21",
        "WIND-21",
        20.0,
        80.0,
        10.0,
        8.0
    };
}

bool hasError(const ManualRouteBuildResult& result, const std::string& expected)
{
    return std::find(result.errors.begin(), result.errors.end(), expected) != result.errors.end();
}

} // namespace

int main()
{
    const Mission mission = missionFixture();
    const MissionGeometryObject geometry = corridorFixture();
    const ManualRouteBuildRequirements requirements = requirementsFixture();

    const auto valid = buildManualRouteCandidate(mission, geometry, requirements);
    assert(valid.valid);
    assert(valid.errors.empty());
    assert(valid.route.lineage.mission_id == mission.identity.mission_id);
    assert(valid.route.lineage.mission_version == mission.identity.mission_version);
    assert(valid.route.lineage.calculation_input_version == "INPUT-7");
    assert(valid.route.waypoints.size() == 3);
    assert(valid.route.segments.size() == 2);
    assert(valid.route.waypoints.front().altitude_reference == AltitudeReference::AGL);
    assert(valid.route.waypoints.front().altitude_m == 40.0);
    assert(valid.route.segments.front().distance_m > 0.0);
    assert(valid.route.segments.front().nominal_speed_mps == 8.0);
    assert(valid.route.environment.terrain_snapshot_id == "TERRAIN-42");

    auto missingAltitudeGeometry = geometry;
    missingAltitudeGeometry.coordinates.front().altitude_m.reset();
    const auto missingAltitude = buildManualRouteCandidate(
        mission, missingAltitudeGeometry, requirements);
    assert(!missingAltitude.valid);
    assert(hasError(missingAltitude, "EXPLICIT_TARGET_ALTITUDE_REQUIRED:0"));

    auto missingDatumGeometry = geometry;
    missingDatumGeometry.coordinates.front().altitude_reference.reset();
    const auto missingDatum = buildManualRouteCandidate(
        mission, missingDatumGeometry, requirements);
    assert(!missingDatum.valid);
    assert(hasError(missingDatum, "EXPLICIT_TARGET_ALTITUDE_REFERENCE_AGL_REQUIRED:0"));

    auto missingEnvironment = requirements;
    missingEnvironment.wind_snapshot_id.clear();
    const auto noWind = buildManualRouteCandidate(mission, geometry, missingEnvironment);
    assert(!noWind.valid);
    assert(hasError(noWind, "WIND_SNAPSHOT_REQUIRED"));

    auto areaGeometry = geometry;
    areaGeometry.geometry_type = "POLYGON_AOI";
    const auto area = buildManualRouteCandidate(mission, areaGeometry, requirements);
    assert(!area.valid);
    assert(hasError(area, "TASK_SPECIFIC_GEOMETRY_GENERATOR_REQUIRED:POLYGON_AOI"));

    auto invalidCoordinateGeometry = geometry;
    invalidCoordinateGeometry.coordinates.front().latitude_deg = 91.0;
    const auto invalidCoordinate = buildManualRouteCandidate(
        mission, invalidCoordinateGeometry, requirements);
    assert(!invalidCoordinate.valid);
    assert(hasError(invalidCoordinate, "GEOMETRY_COORDINATE_INVALID:0"));

    auto duplicatePointGeometry = geometry;
    duplicatePointGeometry.coordinates[1] = duplicatePointGeometry.coordinates[0];
    const auto duplicatePoint = buildManualRouteCandidate(
        mission, duplicatePointGeometry, requirements);
    assert(!duplicatePoint.valid);
    assert(hasError(duplicatePoint, "ROUTE_HAS_ZERO_OR_INVALID_SEGMENT:0"));

    auto unselectedTemplateGeometry = geometry;
    unselectedTemplateGeometry.template_ids = {"MT-04"};
    const auto unselectedTemplate = buildManualRouteCandidate(
        mission, unselectedTemplateGeometry, requirements);
    assert(!unselectedTemplate.valid);
    assert(hasError(unselectedTemplate, "GEOMETRY_TEMPLATE_NOT_SELECTED:MT-04"));

    auto invalidRequirements = requirements;
    invalidRequirements.maximum_target_agl_m = 0.0;
    const auto invalidLimits = buildManualRouteCandidate(
        mission, geometry, invalidRequirements);
    assert(!invalidLimits.valid);
    assert(hasError(invalidLimits, "EXPLICIT_VALID_TARGET_AGL_LIMITS_REQUIRED"));

    return 0;
}
