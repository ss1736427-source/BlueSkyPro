#include "notam_prohibited_zone_validator.hpp"
#include <algorithm>
#include <cmath>

namespace bluesky::planning {
namespace {
constexpr double kEarthRadiusM = 6371000.0;
constexpr double kPi = 3.14159265358979323846;

double deg_to_rad(double v) { return v * kPi / 180.0; }
double normalize_lon_delta(double degrees) {
    while (degrees > 180.0) degrees -= 360.0;
    while (degrees < -180.0) degrees += 360.0;
    return degrees;
}

bool valid_point(const GeoPoint& p) {
    return std::isfinite(p.latitude_deg) && std::isfinite(p.longitude_deg) &&
           p.latitude_deg >= -90.0 && p.latitude_deg <= 90.0 &&
           p.longitude_deg >= -180.0 && p.longitude_deg <= 180.0;
}

double distance_m(const GeoPoint& a, const GeoPoint& b) {
    const double lat1 = deg_to_rad(a.latitude_deg);
    const double lat2 = deg_to_rad(b.latitude_deg);
    const double dlat = lat2 - lat1;
    const double dlon = deg_to_rad(normalize_lon_delta(b.longitude_deg - a.longitude_deg));
    const double h = std::clamp(
        std::sin(dlat / 2.0) * std::sin(dlat / 2.0) +
        std::cos(lat1) * std::cos(lat2) *
        std::sin(dlon / 2.0) * std::sin(dlon / 2.0), 0.0, 1.0);
    return 2.0 * kEarthRadiusM * std::atan2(std::sqrt(h), std::sqrt(1.0 - h));
}

bool point_in_polygon(const GeoPoint& p, const std::vector<GeoPoint>& poly) {
    if (poly.size() < 3) return false;
    bool inside = false;
    for (std::size_t i = 0, j = poly.size() - 1; i < poly.size(); j = i++) {
        const double xi = normalize_lon_delta(poly[i].longitude_deg - p.longitude_deg);
        const double xj = normalize_lon_delta(poly[j].longitude_deg - p.longitude_deg);
        const double yi = poly[i].latitude_deg - p.latitude_deg;
        const double yj = poly[j].latitude_deg - p.latitude_deg;
        const bool crosses = ((yi > 0.0) != (yj > 0.0));
        if (crosses) {
            const double x_at_y = xi + (xj - xi) * (0.0 - yi) / (yj - yi);
            if (0.0 < x_at_y) inside = !inside;
        }
    }
    return inside;
}

struct Point2 { double x; double y; };

Point2 local_xy(const GeoPoint& p, const GeoPoint& origin) {
    const double mean_lat = deg_to_rad((p.latitude_deg + origin.latitude_deg) / 2.0);
    return {kEarthRadiusM * deg_to_rad(normalize_lon_delta(p.longitude_deg - origin.longitude_deg)) *
                std::cos(mean_lat),
            kEarthRadiusM * deg_to_rad(p.latitude_deg - origin.latitude_deg)};
}

double point_segment_distance_m(const GeoPoint& point, const GeoPoint& a, const GeoPoint& b) {
    const auto p = local_xy(point, point);
    const auto aa = local_xy(a, point);
    const auto bb = local_xy(b, point);
    const double dx = bb.x - aa.x, dy = bb.y - aa.y;
    const double length2 = dx * dx + dy * dy;
    if (length2 <= 0.0) return distance_m(point, a);
    const double t = std::clamp(((p.x - aa.x) * dx + (p.y - aa.y) * dy) / length2, 0.0, 1.0);
    const double ex = aa.x + t * dx - p.x;
    const double ey = aa.y + t * dy - p.y;
    return std::sqrt(ex * ex + ey * ey);
}

double cross(const Point2& a, const Point2& b, const Point2& c) {
    return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}

bool on_segment(const Point2& a, const Point2& b, const Point2& p) {
    constexpr double eps = 1e-7;
    return p.x >= std::min(a.x, b.x) - eps && p.x <= std::max(a.x, b.x) + eps &&
           p.y >= std::min(a.y, b.y) - eps && p.y <= std::max(a.y, b.y) + eps;
}

bool segments_intersect(const Point2& a, const Point2& b, const Point2& c, const Point2& d) {
    const double ab_c = cross(a, b, c), ab_d = cross(a, b, d);
    const double cd_a = cross(c, d, a), cd_b = cross(c, d, b);
    if (((ab_c > 0.0 && ab_d < 0.0) || (ab_c < 0.0 && ab_d > 0.0)) &&
        ((cd_a > 0.0 && cd_b < 0.0) || (cd_a < 0.0 && cd_b > 0.0)))
        return true;
    return (std::abs(ab_c) < 1e-7 && on_segment(a, b, c)) ||
           (std::abs(ab_d) < 1e-7 && on_segment(a, b, d)) ||
           (std::abs(cd_a) < 1e-7 && on_segment(c, d, a)) ||
           (std::abs(cd_b) < 1e-7 && on_segment(c, d, b));
}

bool segment_intersects_polygon(const GeoPoint& a, const GeoPoint& b,
                                const std::vector<GeoPoint>& polygon) {
    if (polygon.size() < 3) return false;
    if (point_in_polygon(a, polygon) || point_in_polygon(b, polygon)) return true;
    const auto aa = local_xy(a, a);
    const auto bb = local_xy(b, a);
    for (std::size_t i = 0; i < polygon.size(); ++i) {
        const auto c = local_xy(polygon[i], a);
        const auto d = local_xy(polygon[(i + 1) % polygon.size()], a);
        if (segments_intersect(aa, bb, c, d)) return true;
    }
    return false;
}

bool segment_intersects_zone(const NotamZone& z, const GeoPoint& a, const GeoPoint& b) {
    if (z.geometry_type == NotamGeometryType::Circle)
        return point_segment_distance_m(z.center, a, b) <= z.radius_m;
    return segment_intersects_polygon(a, b, z.polygon);
}

bool valid_zone(const NotamZone& z) {
    if (z.notam_id.empty() || z.component_id.empty() || !std::isfinite(z.lower_altitude_m) ||
        !std::isfinite(z.upper_altitude_m) || z.lower_altitude_m > z.upper_altitude_m ||
        z.valid_from_epoch_s <= 0 || z.valid_to_epoch_s < z.valid_from_epoch_s)
        return false;
    if (z.geometry_type == NotamGeometryType::Circle)
        return valid_point(z.center) && std::isfinite(z.radius_m) && z.radius_m > 0.0;
    if (z.geometry_type == NotamGeometryType::Polygon)
        return z.polygon.size() >= 3 &&
               std::all_of(z.polygon.begin(), z.polygon.end(), valid_point);
    return false;
}

bool contains(const NotamZone& z, const GeoPoint& p) {
    if (z.geometry_type == NotamGeometryType::Circle) return distance_m(z.center, p) <= z.radius_m;
    return point_in_polygon(p, z.polygon);
}

bool active_at_snapshot(const NotamZone& z, std::int64_t epoch) {
    return epoch >= z.valid_from_epoch_s && epoch <= z.valid_to_epoch_s;
}

bool altitude_overlaps(const NotamZone& z, double a, double b) {
    return std::max(std::min(a, b), z.lower_altitude_m) <=
           std::min(std::max(a, b), z.upper_altitude_m);
}
}

NotamValidationResult NotamProhibitedZoneValidator::validate(const Route& route,
                                                              const NotamSnapshot& snapshot) {
    NotamValidationResult result;
    result.snapshot_id = snapshot.snapshot_id;

    const auto reject_snapshot = [&result](const char* detail) {
        result.status = RouteValidationStatus::Rejected;
        result.findings.push_back({NotamValidationCode::SnapshotNotValid, "", "", detail, "", ""});
    };
    const auto reject_altitude_reference = [&result](const char* detail) {
        result.status = RouteValidationStatus::Rejected;
        result.findings.push_back({NotamValidationCode::UnsupportedAltitudeReference,
                                   "", "", detail, "", ""});
    };

    if (route.constraints.avoid_restricted_areas &&
        route.environment.airspace_snapshot_id.empty()) {
        result.status = RouteValidationStatus::Rejected;
        result.findings.push_back({NotamValidationCode::MissingSnapshotReference, "", "",
                                   "Missing airspace/NOTAM snapshot reference", "", ""});
        return result;
    }

    if (snapshot.snapshot_id.empty() || snapshot.evaluation_epoch_s <= 0) {
        reject_snapshot("NOTAM snapshot is missing identity or evaluation time");
        return result;
    }
    if (route.constraints.avoid_restricted_areas &&
        route.environment.airspace_snapshot_id != snapshot.snapshot_id) {
        reject_snapshot("Route airspace snapshot reference does not match supplied NOTAM snapshot");
        return result;
    }

    for (const auto& zone : snapshot.zones) {
        if (!valid_zone(zone)) {
            reject_snapshot("NOTAM snapshot contains an invalid prohibited-zone record");
            return result;
        }
    }

    for (const auto& wp : route.waypoints) {
        if (!valid_point(wp.position) || !std::isfinite(wp.altitude_m)) {
            reject_snapshot("Route contains an invalid waypoint coordinate or altitude");
            return result;
        }
    }

    // Numeric altitude comparisons are safe only when both values share a known datum.
    // Until vertical-datum conversion exists, accept AGL only and fail closed otherwise.
    if (!snapshot.zones.empty()) {
        for (const auto& wp : route.waypoints) {
            if (wp.altitude_reference != AltitudeReference::AGL) {
                reject_altitude_reference("Route waypoint altitude reference is unknown or unsupported; AGL is required");
                return result;
            }
        }
        for (const auto& zone : snapshot.zones) {
            if (zone.altitude_reference != NotamAltitudeReference::AGL) {
                reject_altitude_reference("NOTAM altitude reference is unknown or unsupported; AGL is required");
                return result;
            }
        }
    }

    for (const auto& zone : snapshot.zones) {
        if (zone.restriction_type != NotamRestrictionType::Prohibited ||
            !active_at_snapshot(zone, snapshot.evaluation_epoch_s)) continue;

        for (const auto& wp : route.waypoints) {
            if (wp.altitude_m < zone.lower_altitude_m ||
                wp.altitude_m > zone.upper_altitude_m) continue;
            if (contains(zone, wp.position)) {
                result.status = RouteValidationStatus::Rejected;
                result.findings.push_back({NotamValidationCode::WaypointInsideProhibitedZone,
                                           wp.waypoint_id, zone.notam_id,
                                           "Route waypoint lies inside an active NOTAM prohibited zone",
                                           "", "", zone.component_id});
            }
        }

        for (std::size_t i = 1; i < route.waypoints.size(); ++i) {
            const auto& start = route.waypoints[i - 1];
            const auto& end = route.waypoints[i];
            if (!altitude_overlaps(zone, start.altitude_m, end.altitude_m)) continue;
            if (segment_intersects_zone(zone, start.position, end.position)) {
                result.status = RouteValidationStatus::Rejected;
                result.findings.push_back({NotamValidationCode::SegmentIntersectsProhibitedZone,
                                           "", zone.notam_id,
                                           "Route segment intersects an active NOTAM prohibited zone",
                                           start.waypoint_id, end.waypoint_id, zone.component_id});
            }
        }
    }
    return result;
}


NotamValidationResult NotamProhibitedZoneValidator::validate(
    const Route& route,
    const NotamSnapshot& snapshot,
    const TrajectoryResult& trajectory) {
    // Reuse the existing identity/coordinate gate without applying its
    // snapshot-instant activity rule to a route that has a 4D time profile.
    NotamSnapshot identitySnapshot = snapshot;
    identitySnapshot.zones.clear();
    auto result = validate(route, identitySnapshot);
    result.snapshot_id = snapshot.snapshot_id;
    if (result.status != RouteValidationStatus::Allowed)
        return result;

    const auto reject_timing = [&result](const std::string& detail) {
        result.status = RouteValidationStatus::Rejected;
        result.findings.push_back({NotamValidationCode::TrajectoryTimingUnavailable,
                                   "", "", detail, "", ""});
    };

    if (trajectory.status != TrajectoryStatus::Feasible ||
        trajectory.mission_start_epoch_s <= 0 ||
        trajectory.route_id != route.lineage.route_id ||
        trajectory.route_version != route.lineage.route_version ||
        route.waypoints.empty() ||
        route.segments.size() + 1 != route.waypoints.size() ||
        trajectory.points.size() != route.waypoints.size() ||
        trajectory.segments.size() != route.segments.size() ||
        !std::isfinite(trajectory.total_time_s) ||
        trajectory.total_time_s < 0.0) {
        reject_timing("A feasible 4D timing result with matching route identity/version and UTC mission start is required");
        return result;
    }

    for (const auto& zone : snapshot.zones) {
        if (!valid_zone(zone)) {
            result.status = RouteValidationStatus::Rejected;
            result.findings.push_back({NotamValidationCode::SnapshotNotValid, "", "",
                                       "NOTAM snapshot contains an invalid prohibited-zone record",
                                       "", ""});
            return result;
        }
    }

    if (!snapshot.zones.empty()) {
        for (const auto& waypoint : route.waypoints) {
            if (waypoint.altitude_reference != AltitudeReference::AGL) {
                result.status = RouteValidationStatus::Rejected;
                result.findings.push_back({NotamValidationCode::UnsupportedAltitudeReference,
                                           waypoint.waypoint_id, "",
                                           "Route waypoint altitude reference is unsupported; AGL is required until datum conversion is implemented",
                                           "", ""});
                return result;
            }
        }
        for (const auto& zone : snapshot.zones) {
            if (zone.altitude_reference != NotamAltitudeReference::AGL) {
                result.status = RouteValidationStatus::Rejected;
                result.findings.push_back({NotamValidationCode::UnsupportedAltitudeReference,
                                           "", zone.notam_id,
                                           "NOTAM altitude reference is unsupported; AGL is required until datum conversion is implemented",
                                           "", ""});
                return result;
            }
        }
    }

    constexpr double timingToleranceS = 1e-3;
    for (std::size_t i = 0; i < route.waypoints.size(); ++i) {
        const auto& waypoint = route.waypoints[i];
        const auto& point = trajectory.points[i];
        if (!valid_point(waypoint.position) ||
            !std::isfinite(waypoint.altitude_m) ||
            point.waypoint_id != waypoint.waypoint_id ||
            !valid_point(point.position) ||
            std::abs(point.position.latitude_deg - waypoint.position.latitude_deg) > 1e-9 ||
            std::abs(point.position.longitude_deg - waypoint.position.longitude_deg) > 1e-9 ||
            !std::isfinite(point.altitude_m) ||
            std::abs(point.altitude_m - waypoint.altitude_m) > 1e-6 ||
            !std::isfinite(point.elapsed_time_s) ||
            point.elapsed_time_s < 0.0 ||
            (i > 0 && point.elapsed_time_s + timingToleranceS <
                          trajectory.points[i - 1].elapsed_time_s)) {
            reject_timing("4D trajectory waypoint identity, geometry or elapsed-time ordering does not match the route");
            return result;
        }
    }

    if (std::abs(trajectory.points.back().elapsed_time_s - trajectory.total_time_s) >
        timingToleranceS) {
        reject_timing("4D trajectory total time does not match its final waypoint time");
        return result;
    }

    for (std::size_t i = 0; i < route.segments.size(); ++i) {
        const auto& segment = route.segments[i];
        const auto& timing = trajectory.segments[i];
        const auto& from = route.waypoints[i];
        const auto& to = route.waypoints[i + 1];
        const double departure = trajectory.points[i].elapsed_time_s;
        const double duration = timing.traversal_time_s;
        const double destinationDwell = to.task_execution_duration_s.value_or(0.0);
        const double destinationArrival = trajectory.points[i + 1].elapsed_time_s - destinationDwell;
        if (segment.from_waypoint_id != from.waypoint_id ||
            segment.to_waypoint_id != to.waypoint_id ||
            timing.segment_id != segment.segment_id ||
            !std::isfinite(duration) || duration < 0.0 ||
            destinationArrival + timingToleranceS < departure ||
            std::abs((departure + duration) - destinationArrival) > timingToleranceS) {
            reject_timing("4D segment timing/lineage does not match the ordered route segment chain");
            return result;
        }
    }

    const auto overlaps_validity = [&trajectory](
        const NotamZone& zone, double elapsedStart, double elapsedEnd) {
        const double absoluteStart =
            static_cast<double>(trajectory.mission_start_epoch_s) + elapsedStart;
        const double absoluteEnd =
            static_cast<double>(trajectory.mission_start_epoch_s) + elapsedEnd;
        return absoluteStart <= static_cast<double>(zone.valid_to_epoch_s) &&
               absoluteEnd >= static_cast<double>(zone.valid_from_epoch_s);
    };

    for (const auto& zone : snapshot.zones) {
        if (zone.restriction_type != NotamRestrictionType::Prohibited)
            continue;

        for (std::size_t i = 0; i < route.waypoints.size(); ++i) {
            const auto& waypoint = route.waypoints[i];
            if (waypoint.altitude_m < zone.lower_altitude_m ||
                waypoint.altitude_m > zone.upper_altitude_m ||
                !contains(zone, waypoint.position))
                continue;

            const double departure = trajectory.points[i].elapsed_time_s;
            const double dwell = waypoint.task_execution_duration_s.value_or(0.0);
            const double arrival = departure - dwell;
            if (overlaps_validity(zone, arrival, departure)) {
                result.status = RouteValidationStatus::Rejected;
                result.findings.push_back({
                    NotamValidationCode::WaypointOccupancyOverlapsNotamValidity,
                    waypoint.waypoint_id, zone.notam_id,
                    "Waypoint occupancy interval overlaps the inclusive NOTAM validity interval",
                    "", "", zone.component_id
                });
            }
        }

        for (std::size_t i = 0; i < route.segments.size(); ++i) {
            const auto& from = route.waypoints[i];
            const auto& to = route.waypoints[i + 1];
            if (!altitude_overlaps(zone, from.altitude_m, to.altitude_m) ||
                !segment_intersects_zone(zone, from.position, to.position))
                continue;

            const double segmentStart = trajectory.points[i].elapsed_time_s;
            const double segmentEnd =
                segmentStart + trajectory.segments[i].traversal_time_s;
            if (overlaps_validity(zone, segmentStart, segmentEnd)) {
                result.status = RouteValidationStatus::Rejected;
                result.findings.push_back({
                    NotamValidationCode::SegmentOccupancyOverlapsNotamValidity,
                    "", zone.notam_id,
                    "Segment occupancy interval overlaps the inclusive NOTAM validity interval",
                    from.waypoint_id, to.waypoint_id, zone.component_id
                });
            }
        }
    }
    return result;
}

} // namespace bluesky::planning
