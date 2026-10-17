#pragma once

#include <string>
#include <vector>

namespace bluesky::planning {

struct GeoPoint {
    double latitude_deg{0.0};
    double longitude_deg{0.0};
};

struct RouteWaypoint {
    std::string waypoint_id;
    GeoPoint position;

    // Planned target height above the local ground surface (AGL), in metres.
    // This is not an absolute altitude above mean sea level or an ellipsoid.
    // In flight, the radio altimeter supplies measured AGL feedback separately.
    double altitude_m{0.0};
    bool mandatory{false};
};

struct RouteSegment {
    std::string segment_id;
    std::string from_waypoint_id;
    std::string to_waypoint_id;
    double distance_m{0.0};
    double nominal_speed_mps{0.0};
};

struct RouteConstraints {
    // Bounds for the planned target height above local ground (AGL), in metres.
    double minimum_altitude_m{0.0};
    double maximum_altitude_m{0.0};

    // Minimum required terrain/obstacle clearance, verified by the relevant
    // terrain/obstacle validators; this is not the radio-altimeter measurement.
    double minimum_clearance_m{0.0};
    bool avoid_restricted_areas{true};
};

struct RouteEnvironmentReference {
    std::string terrain_snapshot_id;
    std::string airspace_snapshot_id;
    std::string weather_snapshot_id;
    std::string wind_snapshot_id;
};

struct RouteLineage {
    std::string route_id;
    std::string route_version;
    std::string mission_id;
    std::string mission_version;
    std::string generator_id;
    std::string generator_version;
    std::string calculation_input_version;
};

struct Route {
    RouteLineage lineage;
    std::vector<RouteWaypoint> waypoints;
    std::vector<RouteSegment> segments;
    RouteConstraints constraints;
    RouteEnvironmentReference environment;
};

} // namespace bluesky::planning
