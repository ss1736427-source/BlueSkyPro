#include "constrained_open_space.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

namespace bluesky::planning {
namespace {
constexpr double kMetersPerDegree = 111320.0;
constexpr double kPi = 3.14159265358979323846;

struct XY { double x; double y; };

XY project(const GeoPoint& p, double reference_latitude) {
    return {p.longitude_deg * kMetersPerDegree *
                std::cos(reference_latitude * kPi / 180.0),
            p.latitude_deg * kMetersPerDegree};
}

double cross(XY a, XY b, XY c) {
    return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}

bool onSegment(XY a, XY b, XY p) {
    constexpr double epsilon = 1e-7;
    return std::abs(cross(a, b, p)) < epsilon &&
           p.x >= std::min(a.x, b.x) - epsilon &&
           p.x <= std::max(a.x, b.x) + epsilon &&
           p.y >= std::min(a.y, b.y) - epsilon &&
           p.y <= std::max(a.y, b.y) + epsilon;
}

bool segmentsIntersect(XY a, XY b, XY c, XY d) {
    const double x = cross(a, b, c), y = cross(a, b, d);
    const double z = cross(c, d, a), w = cross(c, d, b);
    if (((x > 0 && y < 0) || (x < 0 && y > 0)) &&
        ((z > 0 && w < 0) || (z < 0 && w > 0))) return true;
    return onSegment(a, b, c) || onSegment(a, b, d) ||
           onSegment(c, d, a) || onSegment(c, d, b);
}

bool insidePolygon(XY p, const std::vector<XY>& vertices) {
    if (vertices.size() < 3) return false;
    bool inside = false;
    for (std::size_t i = 0, j = vertices.size() - 1; i < vertices.size(); j = i++) {
        if ((vertices[i].y > p.y) != (vertices[j].y > p.y)) {
            const double x = vertices[j].x +
                (p.y - vertices[j].y) * (vertices[i].x - vertices[j].x) /
                (vertices[i].y - vertices[j].y);
            if (p.x < x) inside = !inside;
        }
    }
    return inside;
}

double pointToSegmentDistance(XY p, XY a, XY b) {
    const double dx = b.x - a.x, dy = b.y - a.y;
    const double length_squared = dx * dx + dy * dy;
    if (length_squared == 0.0) return std::hypot(p.x - a.x, p.y - a.y);
    const double t = std::max(0.0, std::min(1.0,
        ((p.x - a.x) * dx + (p.y - a.y) * dy) / length_squared));
    return std::hypot(p.x - (a.x + t * dx), p.y - (a.y + t * dy));
}

bool altitudeOverlaps(const SpatialRestriction& restriction, const SpatialEdge& edge) {
    double edge_min = std::isnan(edge.altitude_min_m) ? edge.altitude_m : edge.altitude_min_m;
    double edge_max = std::isnan(edge.altitude_max_m) ? edge.altitude_m : edge.altitude_max_m;
    if (edge_min > edge_max) std::swap(edge_min, edge_max);
    return !(edge_max < restriction.minimum_altitude_m ||
             (restriction.maximum_altitude_m > 0.0 &&
              edge_min > restriction.maximum_altitude_m));
}

bool validCoordinate(const GeoPoint& point) {
    return std::isfinite(point.latitude_deg) && std::isfinite(point.longitude_deg) &&
           point.latitude_deg >= -90.0 && point.latitude_deg <= 90.0 &&
           point.longitude_deg >= -180.0 && point.longitude_deg <= 180.0;
}

bool validRestriction(const SpatialRestriction& restriction) {
    if (!std::isfinite(restriction.minimum_altitude_m) ||
        !std::isfinite(restriction.maximum_altitude_m) ||
        (restriction.maximum_altitude_m > 0.0 &&
         restriction.maximum_altitude_m < restriction.minimum_altitude_m)) {
        return false;
    }
    if (restriction.geometry_type == RestrictionGeometryType::Circle) {
        return validCoordinate(restriction.center) &&
               std::isfinite(restriction.radius_m) && restriction.radius_m >= 0.0;
    }
    if (restriction.geometry_type != RestrictionGeometryType::Polygon ||
        restriction.polygon.size() < 3) return false;
    return std::all_of(restriction.polygon.begin(), restriction.polygon.end(),
                       validCoordinate);
}

bool validEdge(const SpatialEdge& edge) {
    if (!validCoordinate(edge.from) || !validCoordinate(edge.to) ||
        !std::isfinite(edge.altitude_m)) {
        return false;
    }
    const double minimum = std::isnan(edge.altitude_min_m)
        ? edge.altitude_m : edge.altitude_min_m;
    const double maximum = std::isnan(edge.altitude_max_m)
        ? edge.altitude_m : edge.altitude_max_m;
    return std::isfinite(minimum) && std::isfinite(maximum);
}

bool intersects(const SpatialRestriction& restriction, const SpatialEdge& edge) {
    if (!restriction.active || !altitudeOverlaps(restriction, edge)) return false;

    const double latitude = (edge.from.latitude_deg + edge.to.latitude_deg) * 0.5;
    const XY a = project(edge.from, latitude), b = project(edge.to, latitude);
    if (restriction.geometry_type == RestrictionGeometryType::Circle) {
        return pointToSegmentDistance(project(restriction.center, latitude), a, b)
               <= restriction.radius_m;
    }

    if (restriction.polygon.size() < 3) return false;
    std::vector<XY> polygon;
    polygon.reserve(restriction.polygon.size());
    for (const auto& point : restriction.polygon) polygon.push_back(project(point, latitude));
    if (insidePolygon(a, polygon) || insidePolygon(b, polygon)) return true;
    for (std::size_t i = 0; i < polygon.size(); ++i) {
        if (segmentsIntersect(a, b, polygon[i], polygon[(i + 1) % polygon.size()])) return true;
    }
    return false;
}

std::string canonicalDouble(double value) {
    if (std::isnan(value)) return "nan";
    if (std::isinf(value)) return value < 0 ? "-inf" : "+inf";
    std::ostringstream stream;
    stream << std::setprecision(std::numeric_limits<double>::max_digits10) << value;
    return stream.str();
}

void appendPoint(std::ostringstream& out, const GeoPoint& point) {
    out << canonicalDouble(point.latitude_deg) << ',' << canonicalDouble(point.longitude_deg);
}

std::string dependencyIdentity(const ConstrainedEnvironmentSnapshot& environment) {
    std::vector<std::string> restriction_keys;
    restriction_keys.reserve(environment.restrictions.size());
    for (const auto& restriction : environment.restrictions) {
        std::ostringstream key;
        key << restriction.restriction_id << '|' << restriction.source_id << '|'
            << restriction.snapshot_version << '|'
            << static_cast<int>(restriction.geometry_type) << '|'
            << restriction.active << '|'
            << canonicalDouble(restriction.minimum_altitude_m) << '|'
            << canonicalDouble(restriction.maximum_altitude_m) << '|'
            << canonicalDouble(restriction.radius_m) << "|C|";
        appendPoint(key, restriction.center);
        key << "|P|";
        for (const auto& point : restriction.polygon) {
            appendPoint(key, point);
            key << ';';
        }
        restriction_keys.push_back(key.str());
    }
    std::sort(restriction_keys.begin(), restriction_keys.end());

    std::ostringstream canonical;
    canonical << environment.snapshot_id << '|' << environment.snapshot_version << '|'
              << environment.calculation_input_version << '|'
              << environment.complete;
    for (const auto& key : restriction_keys) canonical << "\n" << key;

    // Fixed FNV-1a 64-bit hash; unlike std::hash, this is stable across standard-library
    // implementations. The canonical input uses max_digits10 for floating-point values.
    std::uint64_t hash = UINT64_C(14695981039346656037);
    for (const unsigned char byte : canonical.str()) {
        hash ^= byte;
        hash *= UINT64_C(1099511628211);
    }
    std::ostringstream result;
    result << std::hex << std::setfill('0') << std::setw(16) << hash;
    return result.str();
}

OpenSpaceResult makeResult(const ConstrainedEnvironmentSnapshot& environment) {
    return {true, environment.snapshot_id, environment.snapshot_version,
            environment.calculation_input_version, dependencyIdentity(environment), {}};
}

void reject(OpenSpaceResult& result, const std::string& reason) {
    result.allowed = false;
    result.blocking_restriction_ids.push_back(reason);
}
} // namespace

OpenSpaceResult ConstrainedOpenSpace::evaluateSegment(
    const ConstrainedEnvironmentSnapshot& environment, const SpatialEdge& edge) {
    auto result = makeResult(environment);
    if (!environment.complete) {
        reject(result, "ENVIRONMENT_INCOMPLETE");
        return result;
    }
    if (!validEdge(edge)) {
        reject(result, "INVALID_ROUTE_EDGE");
        return result;
    }
    for (const auto& restriction : environment.restrictions) {
        if (restriction.active && !validRestriction(restriction)) {
            reject(result, "INVALID_RESTRICTION_GEOMETRY:" + restriction.restriction_id);
            return result;
        }
        if (intersects(restriction, edge)) {
            result.blocking_restriction_ids.push_back(restriction.restriction_id);
        }
    }
    std::sort(result.blocking_restriction_ids.begin(), result.blocking_restriction_ids.end());
    result.blocking_restriction_ids.erase(
        std::unique(result.blocking_restriction_ids.begin(), result.blocking_restriction_ids.end()),
        result.blocking_restriction_ids.end());
    result.allowed = result.blocking_restriction_ids.empty();
    return result;
}

OpenSpaceResult ConstrainedOpenSpace::evaluateRoute(
    const ConstrainedEnvironmentSnapshot& environment, const Route& route) {
    auto result = makeResult(environment);
    if (!environment.complete) {
        reject(result, "ENVIRONMENT_INCOMPLETE");
        return result;
    }
    if (route.segments.empty()) {
        reject(result, "ROUTE_HAS_NO_SEGMENTS");
        return result;
    }

    for (const auto& segment : route.segments) {
        const auto from = std::find_if(route.waypoints.begin(), route.waypoints.end(),
            [&](const auto& waypoint) { return waypoint.waypoint_id == segment.from_waypoint_id; });
        const auto to = std::find_if(route.waypoints.begin(), route.waypoints.end(),
            [&](const auto& waypoint) { return waypoint.waypoint_id == segment.to_waypoint_id; });
        if (from == route.waypoints.end() || to == route.waypoints.end()) {
            reject(result, "ROUTE_SEGMENT_MISSING_WAYPOINT:" + segment.segment_id);
            continue;
        }

        if (!std::isfinite(from->altitude_m) || !std::isfinite(to->altitude_m)) {
            reject(result, "INVALID_ROUTE_WAYPOINT_ALTITUDE:" + segment.segment_id);
            continue;
        }

        const double min_altitude = std::min(from->altitude_m, to->altitude_m);
        const double max_altitude = std::max(from->altitude_m, to->altitude_m);
        auto segment_result = evaluateSegment(environment,
            {from->position, to->position, (min_altitude + max_altitude) * 0.5,
             min_altitude, max_altitude});
        result.blocking_restriction_ids.insert(result.blocking_restriction_ids.end(),
            segment_result.blocking_restriction_ids.begin(),
            segment_result.blocking_restriction_ids.end());
    }

    std::sort(result.blocking_restriction_ids.begin(), result.blocking_restriction_ids.end());
    result.blocking_restriction_ids.erase(
        std::unique(result.blocking_restriction_ids.begin(), result.blocking_restriction_ids.end()),
        result.blocking_restriction_ids.end());
    result.allowed = result.blocking_restriction_ids.empty();
    return result;
}
} // namespace bluesky::planning
