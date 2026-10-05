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
    double minimum_altitude_m{0.0};
    double maximum_altitude_m{0.0};
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
