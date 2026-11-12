#pragma once

#include "model/route_model.hpp"
#include "wind_performance_trajectory.hpp"
#include "route_constraint_validator.hpp"
#include <cstdint>
#include <string>
#include <vector>

namespace bluesky::planning {

enum class NotamGeometryType { Circle, Polygon };
enum class NotamRestrictionType { Prohibited };
enum class NotamAltitudeReference { Unknown, AGL, AMSL, FlightLevel };

enum class NotamValidationCode {
    MissingSnapshotReference,
    SnapshotNotValid,
    WaypointInsideProhibitedZone,
    SegmentIntersectsProhibitedZone,
    UnsupportedAltitudeReference,
    TrajectoryTimingUnavailable,
    WaypointOccupancyOverlapsNotamValidity,
    SegmentOccupancyOverlapsNotamValidity,
    TrajectoryOutsideSnapshotWindow
};

struct NotamZone {
    std::string notam_id;
    NotamRestrictionType restriction_type{NotamRestrictionType::Prohibited};
    NotamGeometryType geometry_type{NotamGeometryType::Circle};
    GeoPoint center;
    double radius_m{0.0};
    std::vector<GeoPoint> polygon;
    double lower_altitude_m{0.0};
    double upper_altitude_m{0.0};
    std::int64_t valid_from_epoch_s{0};
    std::int64_t valid_to_epoch_s{0};
    NotamAltitudeReference altitude_reference{NotamAltitudeReference::Unknown};
    // Stable identity of one normalized geometry component within a compound NOTAM.
    std::string component_id;
};

struct NotamSnapshot {
    std::string snapshot_id;
    std::string source_id;
    std::string source_version;
    std::int64_t evaluation_epoch_s{0};
    std::vector<NotamZone> zones;
    GeoPoint coverage_center;
    double coverage_radius_m{0.0};
    // Time interval for which the source records were filtered and verified.
    std::int64_t planned_mission_start_epoch_s{0};
    std::int64_t planned_mission_end_epoch_s{0};
};

struct NotamValidationFinding {
    NotamValidationCode code;
    std::string waypoint_id;
    std::string notam_id;
    std::string detail;
    std::string segment_start_waypoint_id;
    std::string segment_end_waypoint_id;
    std::string component_id;
};

struct NotamValidationResult {
    RouteValidationStatus status{RouteValidationStatus::Allowed};
    std::vector<NotamValidationFinding> findings;
    std::string validator_id{"NOTAM-PROHIBITED-ZONE-VALIDATOR"};
    std::string validator_version{"1.4"};
    std::string snapshot_id;
};

class NotamProhibitedZoneValidator {
public:
    static NotamValidationResult validate(const Route& route,
                                          const NotamSnapshot& snapshot);
    // Final time-dependent gate. Requires the wind/performance 4D result for this exact route version.
    static NotamValidationResult validate(const Route& route,
                                          const NotamSnapshot& snapshot,
                                          const TrajectoryResult& trajectory);
};

} // namespace bluesky::planning
