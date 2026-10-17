#include "route_constraint_validator.hpp"

#include <cassert>
#include <limits>

using namespace bluesky::planning;

static Route base()
{
    Route route;
    route.lineage = {"R-1", "1", "M-1", "1", "generator", "1", "INPUT-1"};
    route.constraints = {80, 120, 30, true};
    route.environment = {"DEM-1", "AIRSPACE-1", "WX-1", "WIND-1"};
    route.waypoints = {
        {"WP-1", {60.0, 25.0}, 100.0, true},
        {"WP-2", {60.1, 25.1}, 100.0, false}
    };
    route.segments = {{"S-1", "WP-1", "WP-2", 1000.0, 20.0}};
    return route;
}

static bool hasCode(const RouteValidationResult& result, RouteFindingCode code)
{
    for (const auto& finding : result.findings) {
        if (finding.code == code) {
            return true;
        }
    }
    return false;
}

int main()
{
    const auto valid = RouteConstraintValidator::validate(base());
    assert(valid.status == RouteValidationStatus::Allowed);
    assert(valid.findings.empty());
    assert(valid.validator_version == "2");
    assert(valid.calculation_input_version == "INPUT-1");

    auto low = base();
    low.waypoints[0].altitude_m = 50.0;
    assert(hasCode(RouteConstraintValidator::validate(low), RouteFindingCode::AltitudeBelowMinimum));

    auto high = base();
    high.waypoints[1].altitude_m = 130.0;
    assert(hasCode(RouteConstraintValidator::validate(high), RouteFindingCode::AltitudeAboveMaximum));

    auto missingReference = base();
    missingReference.segments[0].to_waypoint_id = "MISSING";
    assert(hasCode(RouteConstraintValidator::validate(missingReference), RouteFindingCode::MissingWaypointReference));

    auto disconnected = base();
    disconnected.waypoints.push_back({"WP-3", {60.2, 25.2}, 100.0, false});
    assert(hasCode(RouteConstraintValidator::validate(disconnected), RouteFindingCode::SegmentEndpointMismatch));

    auto missingLineage = base();
    missingLineage.lineage.mission_version.clear();
    assert(hasCode(RouteConstraintValidator::validate(missingLineage), RouteFindingCode::InvalidRouteLineage));

    auto duplicateWaypoint = base();
    duplicateWaypoint.waypoints[1].waypoint_id = duplicateWaypoint.waypoints[0].waypoint_id;
    assert(hasCode(RouteConstraintValidator::validate(duplicateWaypoint), RouteFindingCode::DuplicateWaypointId));

    auto invalidLatitude = base();
    invalidLatitude.waypoints[0].position.latitude_deg = 91.0;
    assert(hasCode(RouteConstraintValidator::validate(invalidLatitude), RouteFindingCode::InvalidWaypointPosition));

    auto nonFiniteAltitude = base();
    nonFiniteAltitude.waypoints[0].altitude_m = std::numeric_limits<double>::quiet_NaN();
    assert(hasCode(RouteConstraintValidator::validate(nonFiniteAltitude), RouteFindingCode::InvalidWaypointAltitude));

    auto invalidLimits = base();
    invalidLimits.constraints.minimum_altitude_m = 130.0;
    invalidLimits.constraints.maximum_altitude_m = 120.0;
    assert(hasCode(RouteConstraintValidator::validate(invalidLimits), RouteFindingCode::InvalidRouteConstraints));

    auto duplicateSegment = base();
    duplicateSegment.segments.push_back(duplicateSegment.segments[0]);
    assert(hasCode(RouteConstraintValidator::validate(duplicateSegment), RouteFindingCode::DuplicateSegmentId));
    assert(hasCode(RouteConstraintValidator::validate(duplicateSegment), RouteFindingCode::DuplicateSegment));

    auto selfLoop = base();
    selfLoop.segments[0].to_waypoint_id = selfLoop.segments[0].from_waypoint_id;
    assert(hasCode(RouteConstraintValidator::validate(selfLoop), RouteFindingCode::SelfReferencingSegment));

    auto invalidSpeed = base();
    invalidSpeed.segments[0].nominal_speed_mps = 0.0;
    assert(hasCode(RouteConstraintValidator::validate(invalidSpeed), RouteFindingCode::InvalidSegmentGeometry));

    auto tooShort = base();
    tooShort.waypoints.resize(1);
    tooShort.segments.clear();
    assert(hasCode(RouteConstraintValidator::validate(tooShort), RouteFindingCode::InsufficientWaypoints));

    return 0;
}
