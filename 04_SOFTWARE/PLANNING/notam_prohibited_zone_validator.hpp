#pragma once

#include "model/route_model.hpp"
#include "route_constraint_validator.hpp"
#include <cstdint>
#include <string>
#include <vector>

namespace bluesky::planning {

enum class NotamGeometryType { Circle, Polygon };
enum class NotamRestrictionType { Prohibited };

enum class NotamValidationCode {
    MissingSnapshotReference,
    SnapshotNotValid,
    WaypointInsideProhibitedZone,
    SegmentIntersectsProhibitedZone
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
};

struct NotamSnapshot {
    std::string snapshot_id;
    std::string source_id;
    std::string source_version;
    std::int64_t evaluation_epoch_s{0};
    std::vector<NotamZone> zones;
};

struct NotamValidationFinding {
    NotamValidationCode code;
    std::string waypoint_id;
    std::string notam_id;
    std::string detail;
    std::string segment_start_waypoint_id;
    std::string segment_end_waypoint_id;
};

struct NotamValidationResult {
    RouteValidationStatus status{RouteValidationStatus::Allowed};
    std::vector<NotamValidationFinding> findings;
    std::string validator_id{"NOTAM-PROHIBITED-ZONE-VALIDATOR"};
    std::string validator_version{"1.0"};
    std::string snapshot_id;
};

class NotamProhibitedZoneValidator {
public:
    static NotamValidationResult validate(const Route& route,
                                          const NotamSnapshot& snapshot);
};

} // namespace bluesky::planning
