#include "constrained_open_space.hpp"

#include <cassert>
#include <cmath>
#include <limits>
#include <string>
using namespace bluesky::planning;

int main() {
    ConstrainedEnvironmentSnapshot environment;
    environment.snapshot_id = "ENV-1";
    environment.snapshot_version = "42";
    environment.calculation_input_version = "calc-42";

    SpatialRestriction restriction;
    restriction.restriction_id = "NOTAM-1";
    restriction.source_id = "NOTAM";
    restriction.snapshot_version = "N-7";
    restriction.polygon = {{59, 30}, {59, 30.01}, {59.01, 30.01}, {59.01, 30}};
    environment.restrictions.push_back(restriction);

    const SpatialEdge crossing{{58.995, 29.995}, {59.005, 30.005}, 100};
    const auto blocked = ConstrainedOpenSpace::evaluateSegment(environment, crossing);
    assert(!blocked.allowed && blocked.blocking_restriction_ids.size() == 1);

    const auto open = ConstrainedOpenSpace::evaluateSegment(
        environment, {{58.99, 29.99}, {58.995, 29.995}, 100});
    assert(open.allowed);

    // Invalid route coordinates and altitudes must fail closed.
    const double nan = std::numeric_limits<double>::quiet_NaN();
    const double infinity = std::numeric_limits<double>::infinity();
    const auto nan_coordinate = ConstrainedOpenSpace::evaluateSegment(
        environment, {{nan, 30.0}, {59.005, 30.005}, 100});
    assert(!nan_coordinate.allowed);
    assert(nan_coordinate.blocking_restriction_ids.front() == "INVALID_ROUTE_EDGE");

    const auto out_of_range_coordinate = ConstrainedOpenSpace::evaluateSegment(
        environment, {{91.0, 30.0}, {59.005, 30.005}, 100});
    assert(!out_of_range_coordinate.allowed);
    assert(out_of_range_coordinate.blocking_restriction_ids.front() == "INVALID_ROUTE_EDGE");

    const auto infinite_altitude = ConstrainedOpenSpace::evaluateSegment(
        environment, {{58.995, 29.995}, {59.005, 30.005}, infinity});
    assert(!infinite_altitude.allowed);
    assert(infinite_altitude.blocking_restriction_ids.front() == "INVALID_ROUTE_EDGE");

    const auto infinite_altitude_envelope = ConstrainedOpenSpace::evaluateSegment(
        environment, {{58.995, 29.995}, {59.005, 30.005}, 100, infinity, 120});
    assert(!infinite_altitude_envelope.allowed);
    assert(infinite_altitude_envelope.blocking_restriction_ids.front() == "INVALID_ROUTE_EDGE");

    auto invalid_altitude_environment = environment;
    invalid_altitude_environment.restrictions[0].minimum_altitude_m = nan;
    const auto invalid_restriction_altitude = ConstrainedOpenSpace::evaluateSegment(
        invalid_altitude_environment, crossing);
    assert(!invalid_restriction_altitude.allowed);
    assert(invalid_restriction_altitude.blocking_restriction_ids.front() ==
           "INVALID_RESTRICTION_GEOMETRY:NOTAM-1");

    // Invalid active restriction geometry must fail closed rather than disappear.
    auto invalid_polygon_environment = environment;
    invalid_polygon_environment.restrictions[0].polygon = {{59, 30}, {59.01, 30.01}};
    const auto invalid_polygon = ConstrainedOpenSpace::evaluateSegment(
        invalid_polygon_environment, crossing);
    assert(!invalid_polygon.allowed);
    assert(invalid_polygon.blocking_restriction_ids.front() ==
           "INVALID_RESTRICTION_GEOMETRY:NOTAM-1");

    auto invalid_circle_environment = environment;
    auto circle = restriction;
    circle.geometry_type = RestrictionGeometryType::Circle;
    circle.center = {59.0, 30.0};
    circle.radius_m = -1.0;
    invalid_circle_environment.restrictions[0] = circle;
    const auto invalid_circle = ConstrainedOpenSpace::evaluateSegment(
        invalid_circle_environment, crossing);
    assert(!invalid_circle.allowed);
    assert(invalid_circle.blocking_restriction_ids.front() ==
           "INVALID_RESTRICTION_GEOMETRY:NOTAM-1");

    auto circle_environment = environment;
    circle.geometry_type = RestrictionGeometryType::Circle;
    circle.center = {59.0, 30.0};
    circle.radius_m = 500.0;
    circle_environment.restrictions[0] = circle;
    const auto circle_blocked = ConstrainedOpenSpace::evaluateSegment(
        circle_environment, crossing);
    assert(!circle_blocked.allowed);

    restriction.minimum_altitude_m = 150;
    environment.restrictions[0] = restriction;
    assert(ConstrainedOpenSpace::evaluateSegment(environment, crossing).allowed);

    restriction.minimum_altitude_m = 0;
    restriction.maximum_altitude_m = 120;
    environment.restrictions[0] = restriction;
    assert(!ConstrainedOpenSpace::evaluateSegment(environment, crossing).allowed);

    const auto envelope = ConstrainedOpenSpace::evaluateSegment(
        environment, {{58.995, 29.995}, {59.005, 30.005}, 150, 80, 160});
    assert(!envelope.allowed);

    environment.complete = false;
    const auto incomplete = ConstrainedOpenSpace::evaluateSegment(environment, crossing);
    assert(!incomplete.allowed &&
           incomplete.blocking_restriction_ids.front() == "ENVIRONMENT_INCOMPLETE");
    environment.complete = true;

    const auto first = ConstrainedOpenSpace::evaluateSegment(
        environment, {{58.99, 29.99}, {58.995, 29.995}, 100});
    const auto second = ConstrainedOpenSpace::evaluateSegment(environment,
        {{58.99, 29.99}, {58.995, 29.995}, 100});
    assert(first.dependency_identity == second.dependency_identity);

    auto changed = environment;
    changed.restrictions[0].polygon[0].latitude_deg += 0.001;
    const auto changed_result = ConstrainedOpenSpace::evaluateSegment(
        changed, {{58.99, 29.99}, {58.995, 29.995}, 100});
    assert(changed_result.dependency_identity != second.dependency_identity);

    Route empty_route;
    const auto empty = ConstrainedOpenSpace::evaluateRoute(environment, empty_route);
    assert(!empty.allowed &&
           empty.blocking_restriction_ids.front() == "ROUTE_HAS_NO_SEGMENTS");

    Route malformed_route;
    malformed_route.segments.push_back({"SEG-1", "MISSING-A", "MISSING-B", 10, 5});
    const auto malformed = ConstrainedOpenSpace::evaluateRoute(environment, malformed_route);
    assert(!malformed.allowed);
    assert(malformed.blocking_restriction_ids.size() == 1);
    assert(malformed.blocking_restriction_ids.front() ==
           "ROUTE_SEGMENT_MISSING_WAYPOINT:SEG-1");

    Route nan_altitude_route;
    nan_altitude_route.waypoints = {
        {"A", {58.99, 29.99}, 100, false},
        {"B", {58.995, 29.995}, nan, false}
    };
    nan_altitude_route.segments.push_back({"SEG-NAN-ALT", "A", "B", 10, 5});
    const auto nan_altitude = ConstrainedOpenSpace::evaluateRoute(environment, nan_altitude_route);
    assert(!nan_altitude.allowed);
    assert(nan_altitude.blocking_restriction_ids.front() ==
           "INVALID_ROUTE_WAYPOINT_ALTITUDE:SEG-NAN-ALT");

    Route infinite_altitude_route;
    infinite_altitude_route.waypoints = {
        {"A", {58.99, 29.99}, 100, false},
        {"B", {58.995, 29.995}, infinity, false}
    };
    infinite_altitude_route.segments.push_back({"SEG-INF-ALT", "A", "B", 10, 5});
    const auto infinite_waypoint_altitude =
        ConstrainedOpenSpace::evaluateRoute(environment, infinite_altitude_route);
    assert(!infinite_waypoint_altitude.allowed);
    assert(infinite_waypoint_altitude.blocking_restriction_ids.front() ==
           "INVALID_ROUTE_WAYPOINT_ALTITUDE:SEG-INF-ALT");

    Route valid_route;
    valid_route.waypoints = {
        {"A", {58.99, 29.99}, 100, false},
        {"B", {58.995, 29.995}, 100, false}
    };
    valid_route.segments.push_back({"SEG-2", "A", "B", 10, 5});
    assert(ConstrainedOpenSpace::evaluateRoute(environment, valid_route).allowed);

    return 0;
}
