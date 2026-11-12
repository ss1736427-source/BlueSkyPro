#include "notam_geometry_adapter.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

namespace bluesky::planning {
namespace {
constexpr double kPi = 3.14159265358979323846;
constexpr double kEarthRadiusM = 6371008.8;
constexpr std::size_t kCircleVertices = 32;

struct XY { double x; double y; };

double radians(double value) { return value * kPi / 180.0; }
bool finite(double value) { return std::isfinite(value); }

bool validPoint(const GeoPoint& point) {
    return finite(point.latitude_deg) && finite(point.longitude_deg)
        && point.latitude_deg >= -90.0 && point.latitude_deg <= 90.0
        && point.longitude_deg >= -180.0 && point.longitude_deg <= 180.0;
}

double normalizeLongitudeDelta(double value) {
    while (value > 180.0) value -= 360.0;
    while (value < -180.0) value += 360.0;
    return value;
}

XY project(const GeoPoint& point, const GeoPoint& origin) {
    const double meanLatitude = radians((point.latitude_deg + origin.latitude_deg) * 0.5);
    return {
        kEarthRadiusM * radians(normalizeLongitudeDelta(point.longitude_deg - origin.longitude_deg))
            * std::cos(meanLatitude),
        kEarthRadiusM * radians(point.latitude_deg - origin.latitude_deg)
    };
}

GeoPoint unproject(const XY& point, const GeoPoint& origin) {
    const double cosine = std::cos(radians(origin.latitude_deg));
    return {
        origin.latitude_deg + point.y / kEarthRadiusM * 180.0 / kPi,
        origin.longitude_deg + point.x / (kEarthRadiusM * cosine) * 180.0 / kPi
    };
}

std::vector<GeoPoint> circlePolygon(const GeoPoint& center, double radiusM) {
    // Circumscribed regular polygon: its edges do not cut inside the requested radius.
    const double vertexRadius = radiusM / std::cos(kPi / static_cast<double>(kCircleVertices));
    std::vector<GeoPoint> ring;
    ring.reserve(kCircleVertices);
    for (std::size_t i = 0; i < kCircleVertices; ++i) {
        const double angle = 2.0 * kPi * static_cast<double>(i) /
                             static_cast<double>(kCircleVertices);
        ring.push_back(unproject({vertexRadius * std::cos(angle),
                                  vertexRadius * std::sin(angle)}, center));
    }
    return ring;
}

bool validZone(const NotamZone& zone) {
    if (zone.notam_id.empty() || zone.component_id.empty() || !finite(zone.lower_altitude_m)
        || !finite(zone.upper_altitude_m) || zone.lower_altitude_m > zone.upper_altitude_m
        || zone.valid_from_epoch_s <= 0 || zone.valid_to_epoch_s < zone.valid_from_epoch_s)
        return false;
    if (zone.geometry_type == NotamGeometryType::Circle)
        return validPoint(zone.center) && finite(zone.radius_m) && zone.radius_m > 0.0;
    if (zone.geometry_type == NotamGeometryType::Polygon)
        return zone.polygon.size() >= 3 &&
               std::all_of(zone.polygon.begin(), zone.polygon.end(), validPoint);
    return false;
}

void appendBufferedPolygon(const std::vector<GeoPoint>& polygon,
                           double bufferM,
                           std::vector<std::vector<GeoPoint>>& output) {
    if (polygon.size() < 3) return;
    output.push_back(polygon);
    if (bufferM <= 0.0) return;

    GeoPoint origin{};
    for (const auto& point : polygon) {
        origin.latitude_deg += point.latitude_deg;
        origin.longitude_deg += point.longitude_deg;
    }
    origin.latitude_deg /= static_cast<double>(polygon.size());
    origin.longitude_deg /= static_cast<double>(polygon.size());
    if (std::abs(origin.latitude_deg) > 85.0) return;

    std::vector<XY> points;
    points.reserve(polygon.size());
    for (const auto& point : polygon) points.push_back(project(point, origin));

    // A conservative polygon buffer is represented as the original polygon,
    // edge rectangles and circumscribed vertex disks. The planner unions overlaps.
    for (std::size_t i = 0; i < points.size(); ++i) {
        const auto& a = points[i];
        const auto& b = points[(i + 1) % points.size()];
        const double dx = b.x - a.x;
        const double dy = b.y - a.y;
        const double length = std::hypot(dx, dy);
        if (length <= 0.01) continue;
        const double nx = -dy / length * bufferM;
        const double ny =  dx / length * bufferM;
        std::vector<GeoPoint> strip{
            unproject({a.x + nx, a.y + ny}, origin),
            unproject({b.x + nx, b.y + ny}, origin),
            unproject({b.x - nx, b.y - ny}, origin),
            unproject({a.x - nx, a.y - ny}, origin)
        };
        output.push_back(std::move(strip));
        output.push_back(circlePolygon(polygon[i], bufferM));
    }
}

} // namespace

NotamGeometryAdapterResult NotamGeometryAdapter::build(
    const NotamGeometryAdapterInput& input) {
    NotamGeometryAdapterResult result;
    const auto fail = [&result](const std::string& reason) {
        result.valid = false;
        result.diagnostics.push_back(reason);
        return result;
    };

    if (!input.snapshot)
        return fail("NOTAM_SNAPSHOT_REQUIRED: route geometry generation cannot proceed without an authoritative NOTAM snapshot.");
    const auto& snapshot = *input.snapshot;
    result.snapshot_id = snapshot.snapshot_id;
    if (snapshot.snapshot_id.empty())
        return fail("NOTAM_SNAPSHOT_REQUIRED: an identified authoritative snapshot is mandatory.");
    if (snapshot.source_id.empty() || snapshot.source_version.empty()
        || snapshot.evaluation_epoch_s <= 0)
        return fail("NOTAM_SNAPSHOT_IDENTITY_INVALID: source ID/version and evaluation epoch are mandatory.");
    if (input.mission_start_epoch_s <= 0 ||
        input.mission_end_epoch_s < input.mission_start_epoch_s)
        return fail("NOTAM_MISSION_TIME_WINDOW_INVALID: explicit planned mission start and end UTC epochs are required.");
    if (!finite(input.target_altitude_agl_m) || input.target_altitude_agl_m < 0.0)
        return fail("NOTAM_TARGET_ALTITUDE_INVALID: a finite AGL target altitude is required.");
    if (!finite(input.safety_buffer_m) || input.safety_buffer_m < 0.0)
        return fail("NOTAM_SAFETY_BUFFER_INVALID: safety buffer must be finite and non-negative.");
    if (!validPoint(snapshot.coverage_center) || !finite(snapshot.coverage_radius_m)
        || snapshot.coverage_radius_m <= 0.0)
        return fail("NOTAM_SNAPSHOT_COVERAGE_METADATA_REQUIRED: source query center and positive radius are mandatory.");
    if (input.aoi_boundary.size() < 3
        || !std::all_of(input.aoi_boundary.begin(), input.aoi_boundary.end(), validPoint))
        return fail("NOTAM_AOI_BOUNDARY_INVALID: a valid AOI boundary is required to prove source coverage.");

    for (const auto& vertex : input.aoi_boundary) {
        const double lat1 = radians(snapshot.coverage_center.latitude_deg);
        const double lat2 = radians(vertex.latitude_deg);
        const double dLat = lat2 - lat1;
        const double dLon = radians(normalizeLongitudeDelta(
            vertex.longitude_deg - snapshot.coverage_center.longitude_deg));
        const double sinLat = std::sin(dLat * 0.5);
        const double sinLon = std::sin(dLon * 0.5);
        const double haversine = sinLat * sinLat
            + std::cos(lat1) * std::cos(lat2) * sinLon * sinLon;
        const double distanceM = 2.0 * kEarthRadiusM
            * std::asin(std::sqrt(std::clamp(haversine, 0.0, 1.0)));
        if (!finite(distanceM)
            || distanceM + input.safety_buffer_m > snapshot.coverage_radius_m + 0.01)
            return fail("NOTAM_SNAPSHOT_DOES_NOT_COVER_AOI_WITH_BUFFER: FAA NMS query radius does not contain the complete AOI plus the configured safety buffer.");
    }

    for (const auto& zone : snapshot.zones) {
        if (!validZone(zone))
            return fail("NOTAM_ZONE_INVALID: snapshot contains a malformed geometry, altitude band or validity interval.");
        if (zone.restriction_type != NotamRestrictionType::Prohibited)
            continue;

        const bool timeApplicable =
            zone.valid_from_epoch_s <= input.mission_end_epoch_s &&
            zone.valid_to_epoch_s >= input.mission_start_epoch_s;
        if (!timeApplicable) continue;

        if (zone.altitude_reference != NotamAltitudeReference::AGL)
            return fail("NOTAM_ALTITUDE_DATUM_UNSUPPORTED: time-applicable NOTAM " +
                        zone.notam_id + " is not expressed in AGL; geometry generation is blocked.");

        const bool altitudeApplicable =
            input.target_altitude_agl_m >= zone.lower_altitude_m &&
            input.target_altitude_agl_m <= zone.upper_altitude_m;
        if (!altitudeApplicable) continue;

        if (input.safety_buffer_m <= 0.0)
            return fail("NOTAM_SAFETY_BUFFER_REQUIRED: applicable prohibited NOTAM " +
                        zone.notam_id + " requires an explicit positive safety buffer.");

        if (zone.geometry_type == NotamGeometryType::Circle) {
            result.exclusion_areas.push_back(
                circlePolygon(zone.center, zone.radius_m + input.safety_buffer_m));
        } else {
            appendBufferedPolygon(zone.polygon, input.safety_buffer_m,
                                   result.exclusion_areas);
        }
        result.applicable_notam_ids.push_back(zone.notam_id);
        result.applicable_component_ids.push_back(zone.component_id);
    }

    result.applicable_zone_count = result.applicable_notam_ids.size();
    result.valid = true;
    result.diagnostics.push_back(
        "NOTAM_GEOMETRY_BOUND: source query coverage contains the complete AOI plus safety buffer; applicable prohibited zones were filtered by planned time and target AGL, buffered and converted to coverage/turn exclusions.");
    return result;
}

} // namespace bluesky::planning
