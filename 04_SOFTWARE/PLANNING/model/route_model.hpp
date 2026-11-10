#pragma once

#include <optional>
#include <string>
#include <vector>

namespace bluesky::planning {

// Route altitude datum must be explicit before comparing it with airspace limits.
enum class AltitudeReference { Unknown, AGL, AMSL, FlightLevel };

struct GeoPoint {
    double latitude_deg{0.0};
    double longitude_deg{0.0};
};

struct RouteWaypoint {
    std::string waypoint_id;
    GeoPoint position;
    // Planned target height above the local ground surface (AGL), in metres.
    // This is not AMSL/ellipsoidal altitude or a live radio-altimeter reading.
    double altitude_m{0.0};
    bool mandatory{false};
    // The current route validator accepts only explicit AGL; other datums need conversion.
    AltitudeReference altitude_reference{AltitudeReference::Unknown};
    // Optional validated task execution at this waypoint. Both values must be
    // supplied together; energy is sourced from the assigned UAV/payload model,
    // never inferred from a generic hover assumption.
    std::optional<double> task_execution_duration_s;
    std::optional<double> task_execution_energy_wh;
};

struct RouteSegment {
    std::string segment_id;
    std::string from_waypoint_id;
    std::string to_waypoint_id;
    double distance_m{0.0};
    double nominal_speed_mps{0.0};
    // Required by the shared dynamic safety gate for the applicable vehicle class.
    std::optional<double> bank_angle_rad;
    std::optional<double> lateral_acceleration_mps2;
};

struct RouteConstraints {
    // Bounds for the planned target height above local ground (AGL), in metres.
    double minimum_altitude_m{0.0};
    // Zero means no configured upper bound; negative values are invalid.
    double maximum_altitude_m{0.0};
    // Minimum terrain/obstacle clearance; distinct from target AGL height.
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
    // True only when this Route includes every flown leg and transition/turn segment.
    // Structural waypoint chains alone must leave this false.
    bool trajectory_geometry_complete{false};
    RouteConstraints constraints;
    RouteEnvironmentReference environment;
};

} // namespace bluesky::planning
