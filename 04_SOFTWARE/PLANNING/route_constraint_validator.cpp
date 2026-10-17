#include "route_constraint_validator.hpp"

#include <algorithm>
#include <cmath>
#include <iterator>
#include <set>
#include <string>
#include <unordered_map>

namespace bluesky::planning {
namespace {

bool isFinite(double value)
{
    return std::isfinite(value);
}

void addFinding(RouteValidationResult& result,
                RouteFindingCode code,
                const std::string& objectId,
                const std::string& detail)
{
    result.findings.push_back({code, objectId, detail});
}

bool hasRequiredLineage(const RouteLineage& lineage)
{
    return !lineage.route_id.empty()
        && !lineage.route_version.empty()
        && !lineage.mission_id.empty()
        && !lineage.mission_version.empty()
        && !lineage.generator_id.empty()
        && !lineage.generator_version.empty()
        && !lineage.calculation_input_version.empty();
}

} // namespace

RouteValidationResult RouteConstraintValidator::validate(const Route& route)
{
    RouteValidationResult result;
    result.validator_id = "canonical-route-constraint-validator";
    result.validator_version = "3";
    result.calculation_input_version = route.lineage.calculation_input_version;

    if (!hasRequiredLineage(route.lineage)) {
        addFinding(result, RouteFindingCode::InvalidRouteLineage, route.lineage.route_id,
                   "route, mission, generator and calculation-input lineage fields are required");
    }

    const auto& constraints = route.constraints;
    if (!isFinite(constraints.minimum_altitude_m)
        || !isFinite(constraints.maximum_altitude_m)
        || !isFinite(constraints.minimum_clearance_m)
        || constraints.minimum_clearance_m < 0.0
        || (constraints.maximum_altitude_m > 0.0
            && constraints.minimum_altitude_m > constraints.maximum_altitude_m)) {
        addFinding(result, RouteFindingCode::InvalidRouteConstraints, route.lineage.route_id,
                   "route altitude limits and minimum clearance must be finite and internally consistent");
    }

    if (route.waypoints.size() < 2) {
        addFinding(result, RouteFindingCode::InsufficientWaypoints, route.lineage.route_id,
                   "a route must contain at least two ordered waypoints");
    }

    std::unordered_map<std::string, bool> waypointIds;
    waypointIds.reserve(route.waypoints.size());

    for (const auto& waypoint : route.waypoints) {
        if (waypoint.waypoint_id.empty()) {
            addFinding(result, RouteFindingCode::InvalidWaypointId, waypoint.waypoint_id,
                       "waypoint identifier must not be empty");
        } else if (!waypointIds.emplace(waypoint.waypoint_id, true).second) {
            addFinding(result, RouteFindingCode::DuplicateWaypointId, waypoint.waypoint_id,
                       "waypoint identifiers must be unique within a route");
        }

        if (!isFinite(waypoint.position.latitude_deg)
            || !isFinite(waypoint.position.longitude_deg)
            || waypoint.position.latitude_deg < -90.0
            || waypoint.position.latitude_deg > 90.0
            || waypoint.position.longitude_deg < -180.0
            || waypoint.position.longitude_deg > 180.0) {
            addFinding(result, RouteFindingCode::InvalidWaypointPosition, waypoint.waypoint_id,
                       "geodetic position must use finite latitude [-90, 90] and longitude [-180, 180]");
        }

        if (!isFinite(waypoint.altitude_m)) {
            addFinding(result, RouteFindingCode::InvalidWaypointAltitude, waypoint.waypoint_id,
                       "waypoint altitude must be finite");
        } else {
            if (waypoint.altitude_m < constraints.minimum_altitude_m) {
                addFinding(result, RouteFindingCode::AltitudeBelowMinimum, waypoint.waypoint_id,
                           "waypoint altitude below route minimum");
            }
            if (constraints.maximum_altitude_m > 0.0
                && waypoint.altitude_m > constraints.maximum_altitude_m) {
                addFinding(result, RouteFindingCode::AltitudeAboveMaximum, waypoint.waypoint_id,
                           "waypoint altitude above route maximum");
            }
        }
    }

    std::set<std::string> segmentIds;
    std::set<std::pair<std::string, std::string>> segmentEndpoints;
    std::unordered_map<std::string, std::size_t> connectionCounts;
    connectionCounts.reserve(route.waypoints.size());

    for (const auto& segment : route.segments) {
        if (segment.segment_id.empty()) {
            addFinding(result, RouteFindingCode::InvalidSegmentId, segment.segment_id,
                       "segment identifier must not be empty");
        } else if (!segmentIds.insert(segment.segment_id).second) {
            addFinding(result, RouteFindingCode::DuplicateSegmentId, segment.segment_id,
                       "segment identifiers must be unique within a route");
        }

        const bool sourceExists = waypointIds.find(segment.from_waypoint_id) != waypointIds.end();
        const bool targetExists = waypointIds.find(segment.to_waypoint_id) != waypointIds.end();
        if (!sourceExists) {
            addFinding(result, RouteFindingCode::MissingWaypointReference, segment.segment_id,
                       "segment source waypoint not found");
        }
        if (!targetExists) {
            addFinding(result, RouteFindingCode::MissingWaypointReference, segment.segment_id,
                       "segment destination waypoint not found");
        }

        if (segment.from_waypoint_id == segment.to_waypoint_id) {
            addFinding(result, RouteFindingCode::SelfReferencingSegment, segment.segment_id,
                       "segment source and destination must be different waypoints");
        }

        if (!isFinite(segment.distance_m) || segment.distance_m < 0.0
            || !isFinite(segment.nominal_speed_mps) || segment.nominal_speed_mps <= 0.0) {
            addFinding(result, RouteFindingCode::InvalidSegmentGeometry, segment.segment_id,
                       "segment distance must be finite and non-negative; nominal speed must be finite and positive");
        }

        if (sourceExists && targetExists && segment.from_waypoint_id != segment.to_waypoint_id) {
            if (!segmentEndpoints.emplace(segment.from_waypoint_id, segment.to_waypoint_id).second) {
                addFinding(result, RouteFindingCode::DuplicateSegment, segment.segment_id,
                           "duplicate directed segment between the same waypoint pair");
            }
            ++connectionCounts[segment.from_waypoint_id];
            ++connectionCounts[segment.to_waypoint_id];
        }
    }

    if (route.waypoints.size() > 1) {
        for (const auto& waypoint : route.waypoints) {
            if (connectionCounts[waypoint.waypoint_id] == 0) {
                addFinding(result, RouteFindingCode::SegmentEndpointMismatch, waypoint.waypoint_id,
                           "waypoint is not connected to a route segment");
            }
        }

        for (std::size_t index = 0; index + 1 < route.waypoints.size(); ++index) {
            const auto& from = route.waypoints[index].waypoint_id;
            const auto& to = route.waypoints[index + 1].waypoint_id;
            if (segmentEndpoints.find({from, to}) == segmentEndpoints.end()) {
                addFinding(result, RouteFindingCode::SegmentOrderMismatch, from,
                           "route must contain a directed segment from each waypoint to the next waypoint in canonical order");
            }
        }

        for (const auto& segment : route.segments) {
            const auto source = std::find_if(route.waypoints.begin(), route.waypoints.end(),
                [&](const RouteWaypoint& waypoint) { return waypoint.waypoint_id == segment.from_waypoint_id; });
            const auto target = std::find_if(route.waypoints.begin(), route.waypoints.end(),
                [&](const RouteWaypoint& waypoint) { return waypoint.waypoint_id == segment.to_waypoint_id; });
            if (source == route.waypoints.end() || target == route.waypoints.end()) {
                continue;
            }
            if (std::distance(route.waypoints.begin(), target)
                != std::distance(route.waypoints.begin(), source) + 1) {
                addFinding(result, RouteFindingCode::SegmentOrderMismatch, segment.segment_id,
                           "segment must connect consecutive waypoints in forward canonical route order");
            }
        }
    }

    if (!result.findings.empty()) {
        result.status = RouteValidationStatus::Rejected;
    }
    return result;
}

} // namespace bluesky::planning
