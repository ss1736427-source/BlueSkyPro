#include "mt01_kinematic_geometry_builder.hpp"

#include <algorithm>
#include <cmath>
#include <utility>

namespace bluesky::planning {
namespace {
constexpr double kPi = 3.14159265358979323846;
constexpr double kEarthRadiusM = 6371008.8;

struct XY { double x; double y; };

bool finite(double value) { return std::isfinite(value); }

XY project(const GeoPoint& point, double origin_lat, double origin_lon)
{
    const double lat = origin_lat * kPi / 180.0;
    const double cosine = std::cos(lat);
    return {
        kEarthRadiusM * (point.longitude_deg - origin_lon) * kPi / 180.0 * cosine,
        kEarthRadiusM * (point.latitude_deg - origin_lat) * kPi / 180.0
    };
}

GeoPoint unproject(const TurnPose2D& point, double origin_lat, double origin_lon)
{
    const double cosine = std::cos(origin_lat * kPi / 180.0);
    return {
        origin_lat + point.y_m / kEarthRadiusM * 180.0 / kPi,
        origin_lon + point.x_m / (kEarthRadiusM * cosine) * 180.0 / kPi
    };
}

bool validGeo(const GeoPoint& point)
{
    return finite(point.latitude_deg) && point.latitude_deg >= -90.0
        && point.latitude_deg <= 90.0 && finite(point.longitude_deg)
        && point.longitude_deg >= -180.0 && point.longitude_deg <= 180.0;
}

} // namespace

Mt01KinematicGeometryResult Mt01KinematicGeometryBuilder::build(
    const CoveragePlanResult& coverage,
    const VehicleMotionModel& vehicle,
    const std::string& assignedVehicleId,
    const std::string& assignedConfigurationId,
    double coverageCruiseSpeedMps,
    const std::vector<std::vector<GeoPoint>>& exclusionAreas,
    double maximumSampleSpacingM)
{
    Mt01KinematicGeometryResult result;
    result.vehicle_model_id = vehicle.model_id;
    result.vehicle_model_version = vehicle.model_version;
    result.vehicle_configuration_id = vehicle.configuration_id;

    const auto fail = [&](const std::string& reason) {
        result.feasible = false;
        result.waypoints.clear();
        result.segments.clear();
        result.diagnostics.push_back(reason);
        return result;
    };

    if (!coverage.valid || coverage.waypoints.size() < 2
        || coverage.waypoints.size() % 2 != 0) {
        return fail("MT01_KINEMATIC_COVERAGE_INPUT_INVALID: expected valid ordered start/end pairs for every coverage leg.");
    }
    if (vehicle.motion_class != MotionClass::FixedWing) {
        return fail("MT01_KINEMATIC_CONNECTOR_UNAVAILABLE: this builder supports fixed-wing Dubins turns only; multirotor transitions require a separately validated dynamics model.");
    }
    if (vehicle.model_id.empty() || vehicle.model_id != assignedVehicleId
        || vehicle.configuration_id.empty()
        || vehicle.configuration_id != assignedConfigurationId) {
        return fail("MT01_VEHICLE_MODEL_BINDING_MISMATCH: exact assigned vehicle and configuration identity are required.");
    }
    if (!finite(coverageCruiseSpeedMps) || coverageCruiseSpeedMps <= 0.0
        || !finite(vehicle.reference_speed_mps)
        || std::abs(coverageCruiseSpeedMps - vehicle.reference_speed_mps) > 1e-6) {
        return fail("MT01_TURN_SPEED_MISMATCH: coverage-leg speed must equal the validated vehicle reference TAS used to derive turn radius.");
    }
    if (!finite(maximumSampleSpacingM) || maximumSampleSpacingM <= 0.0) {
        return fail("MT01_TURN_SAMPLE_SPACING_INVALID: a finite positive sample spacing is required.");
    }

    const auto radius = VehicleMotionModelEvaluator::estimateTurnRadius(vehicle);
    if (!radius.valid || !radius.may_satisfy_motion_hard_gate) {
        return fail("MT01_TURN_RADIUS_NOT_VALIDATED: " + radius.diagnostic);
    }
    result.minimum_turn_radius_m = radius.air_relative_radius_m;

    double originLat = 0.0;
    double originLon = 0.0;
    for (const auto& point : coverage.waypoints) {
        if (!validGeo(point))
            return fail("MT01_COVERAGE_GEOMETRY_INVALID: coverage waypoint is outside WGS84 bounds.");
        originLat += point.latitude_deg;
        originLon += point.longitude_deg;
    }
    originLat /= static_cast<double>(coverage.waypoints.size());
    originLon /= static_cast<double>(coverage.waypoints.size());
    if (std::abs(originLat) > 85.0 || std::abs(std::cos(originLat * kPi / 180.0)) < 1e-6)
        return fail("MT01_LOCAL_PROJECTION_UNSUPPORTED: AOI latitude is outside the supported range.");

    std::vector<TurnPolygon2D> prohibited;
    prohibited.reserve(exclusionAreas.size());
    for (const auto& ring : exclusionAreas) {
        if (ring.size() < 3)
            return fail("MT01_EXCLUSION_GEOMETRY_INVALID: every exclusion polygon requires at least three vertices.");
        TurnPolygon2D polygon;
        polygon.vertices.reserve(ring.size());
        for (const auto& point : ring) {
            if (!validGeo(point))
                return fail("MT01_EXCLUSION_GEOMETRY_INVALID: exclusion coordinate is outside WGS84 bounds.");
            const auto xy = project(point, originLat, originLon);
            polygon.vertices.push_back({xy.x, xy.y, 0.0});
        }
        prohibited.push_back(std::move(polygon));
    }

    const auto appendInitial = [&](const GeoPoint& point) {
        if (result.waypoints.empty())
            result.waypoints.push_back(point);
    };
    const auto appendEdge = [&](const GeoPoint& point,
                                Mt01KinematicSegmentType type,
                                double bankAngleRad,
                                double lateralAccelerationMps2) {
        if (result.waypoints.empty()) {
            result.waypoints.push_back(point);
            return;
        }
        const auto& previous = result.waypoints.back();
        const auto delta = project(point, previous.latitude_deg, previous.longitude_deg);
        const double distance = std::hypot(delta.x, delta.y);
        if (distance <= 0.01)
            return;
        const std::size_t fromIndex = result.waypoints.size() - 1;
        result.waypoints.push_back(point);
        Mt01KinematicSegment segment;
        segment.segment_id = "MT01-SEG-" + std::to_string(result.segments.size() + 1);
        segment.from_waypoint_index = fromIndex;
        segment.to_waypoint_index = fromIndex + 1;
        segment.type = type;
        segment.distance_m = distance;
        segment.nominal_speed_mps = coverageCruiseSpeedMps;
        segment.bank_angle_rad = bankAngleRad;
        segment.lateral_acceleration_mps2 = lateralAccelerationMps2;
        result.segments.push_back(std::move(segment));
    };

    for (std::size_t leg = 0; leg < coverage.waypoints.size(); leg += 2) {
        const auto startXY = project(coverage.waypoints[leg], originLat, originLon);
        const auto finishXY = project(coverage.waypoints[leg + 1], originLat, originLon);
        const double dx = finishXY.x - startXY.x;
        const double dy = finishXY.y - startXY.y;
        if (std::hypot(dx, dy) < 0.1)
            return fail("MT01_COVERAGE_LEG_DEGENERATE: every coverage leg must have a measurable length.");

        const double heading = std::atan2(dy, dx);
        const TurnPose2D start{startXY.x, startXY.y, heading};
        const TurnPose2D finish{finishXY.x, finishXY.y, heading};

        if (leg == 0) {
            appendInitial(coverage.waypoints[leg]);
        } else {
            const auto& previousGeo = result.waypoints.back();
            const auto previousXY = project(previousGeo, originLat, originLon);
            const auto previousBefore = project(result.waypoints[result.waypoints.size() - 2],
                                                originLat, originLon);
            const double previousHeading = std::atan2(
                previousXY.y - previousBefore.y, previousXY.x - previousBefore.x);
            TurnConnectorInput connectorInput;
            connectorInput.start = {previousXY.x, previousXY.y, previousHeading};
            connectorInput.finish = start;
            connectorInput.minimum_turn_radius_m = radius.air_relative_radius_m;
            connectorInput.maximum_sample_spacing_m = maximumSampleSpacingM;
            connectorInput.prohibited_areas = prohibited;
            const auto connector = DubinsTurnConnector::build(connectorInput);
            if (!connector.feasible)
                return fail("MT01_TURN_CONNECTOR_BLOCKED: " + connector.diagnostic);
            for (std::size_t i = 1; i < connector.points.size(); ++i) {
                const double headingDelta = std::abs(std::remainder(
                    connector.points[i].heading_rad - connector.points[i - 1].heading_rad,
                    2.0 * kPi));
                const bool turning = headingDelta > 1e-7;
                appendEdge(
                    unproject(connector.points[i], originLat, originLon),
                    turning ? Mt01KinematicSegmentType::ConnectorTurn
                            : Mt01KinematicSegmentType::ConnectorStraight,
                    turning ? vehicle.bank_angle_rad : 0.0,
                    turning ? (coverageCruiseSpeedMps * coverageCruiseSpeedMps /
                               radius.air_relative_radius_m) : 0.0);
            }
            ++result.connector_count;
        }
        appendEdge(coverage.waypoints[leg + 1],
                   Mt01KinematicSegmentType::Coverage, 0.0, 0.0);
    }

    if (result.waypoints.size() < 2
        || result.segments.size() + 1 != result.waypoints.size())
        return fail("MT01_KINEMATIC_GEOMETRY_EMPTY: waypoint/segment chain is incomplete.");

    for (std::size_t i = 0; i < result.segments.size(); ++i) {
        const auto& segment = result.segments[i];
        if (segment.segment_id.empty()
            || segment.from_waypoint_index != i
            || segment.to_waypoint_index != i + 1
            || !finite(segment.distance_m) || segment.distance_m <= 0.0
            || !finite(segment.nominal_speed_mps) || segment.nominal_speed_mps <= 0.0
            || !finite(segment.bank_angle_rad) || segment.bank_angle_rad < 0.0
            || !finite(segment.lateral_acceleration_mps2)
            || segment.lateral_acceleration_mps2 < 0.0) {
            return fail("MT01_KINEMATIC_SEGMENT_INVALID: every generated edge must carry finite ordered dynamics.");
        }
    }

    result.feasible = true;
    result.diagnostics.push_back(
        "MT01_KINEMATIC_AIR_RELATIVE_CANDIDATE: fixed-wing Dubins connectors were built from validated TAS/bank evidence; this is not a ground-track or flight-release validation.");
    result.diagnostics.push_back(
        "MT01_FLIGHT_RELEASE_STILL_BLOCKED: wind correction, DEM/terrain clearance, obstacle/NOTAM/airspace checks, segment-level dynamics, energy/reserve and complete route attestation remain required.");
    return result;
}

} // namespace bluesky::planning
