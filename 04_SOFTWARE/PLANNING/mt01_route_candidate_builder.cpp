#include "mt01_route_candidate_builder.hpp"

#include <algorithm>
#include <cmath>
#include <string>
#include <utility>

namespace bluesky::planning {

Mt01RouteCandidateResult Mt01RouteCandidateBuilder::build(
    const Mt01KinematicGeometryResult& geometry,
    const CoveragePlanResult& coverage,
    const std::string& missionId,
    const std::string& missionVersion,
    const Mt01RouteCandidateMetadata& metadata,
    const SpeedProfileResult* speedProfile)
{
    Mt01RouteCandidateResult result;
    const auto fail = [&](const std::string& reason) {
        result.available = false;
        result.route = Route{};
        result.diagnostics.push_back(reason);
        return result;
    };

    if (!geometry.feasible || geometry.waypoints.size() < 2
        || geometry.segments.size() + 1 != geometry.waypoints.size()) {
        return fail("MT01_ROUTE_CANDIDATE_GEOMETRY_INCOMPLETE: a feasible, ordered waypoint/segment chain is required.");
    }
    if (!coverage.valid || !std::isfinite(coverage.metrics.calculated_altitude_agl_m)
        || coverage.metrics.calculated_altitude_agl_m < 0.0) {
        return fail("MT01_ROUTE_CANDIDATE_ALTITUDE_UNAVAILABLE: a finite non-negative camera-derived AGL target is required.");
    }
    if (missionId.empty() || missionVersion.empty()
        || metadata.route_id.empty() || metadata.route_version.empty()
        || metadata.generator_id.empty() || metadata.generator_version.empty()
        || metadata.calculation_input_version.empty()) {
        return fail("MT01_ROUTE_CANDIDATE_LINEAGE_INCOMPLETE: mission, route, generator and calculation-input identities are mandatory.");
    }
    if (!std::isfinite(metadata.constraints.minimum_altitude_m)
        || !std::isfinite(metadata.constraints.maximum_altitude_m)
        || !std::isfinite(metadata.constraints.minimum_clearance_m)
        || metadata.constraints.minimum_altitude_m < 0.0
        || metadata.constraints.maximum_altitude_m < 0.0
        || metadata.constraints.minimum_clearance_m < 0.0
        || (metadata.constraints.maximum_altitude_m > 0.0
            && metadata.constraints.minimum_altitude_m > metadata.constraints.maximum_altitude_m)) {
        return fail("MT01_ROUTE_CANDIDATE_CONSTRAINTS_INVALID: target AGL bounds and terrain/obstacle clearance must be finite and consistent.");
    }
    const double altitude = coverage.metrics.calculated_altitude_agl_m;
    if (altitude < metadata.constraints.minimum_altitude_m
        || (metadata.constraints.maximum_altitude_m > 0.0
            && altitude > metadata.constraints.maximum_altitude_m)) {
        return fail("MT01_ROUTE_CANDIDATE_ALTITUDE_OUTSIDE_LIMITS: camera-derived AGL target violates mission constraints.");
    }

    auto& route = result.route;
    route.lineage.route_id = metadata.route_id;
    route.lineage.route_version = metadata.route_version;
    route.lineage.mission_id = missionId;
    route.lineage.mission_version = missionVersion;
    route.lineage.generator_id = metadata.generator_id;
    route.lineage.generator_version = metadata.generator_version;
    route.lineage.calculation_input_version = metadata.calculation_input_version;
    route.constraints = metadata.constraints;
    route.environment = metadata.environment;
    route.trajectory_geometry_complete = false;

    route.waypoints.reserve(geometry.waypoints.size());
    for (std::size_t i = 0; i < geometry.waypoints.size(); ++i) {
        RouteWaypoint waypoint;
        waypoint.waypoint_id = "MT01-WP-" + std::to_string(i + 1);
        waypoint.position = geometry.waypoints[i];
        waypoint.altitude_m = altitude;
        waypoint.mandatory = i == 0 || i + 1 == geometry.waypoints.size();
        waypoint.altitude_reference = AltitudeReference::AGL;
        route.waypoints.push_back(std::move(waypoint));
    }

    if (speedProfile) {
        if (speedProfile->status != SpeedProfileStatus::Feasible
            || speedProfile->vehicle_model_id.empty()
            || speedProfile->vehicle_model_id != geometry.vehicle_model_id
            || speedProfile->vehicle_model_version.empty()
            || speedProfile->vehicle_model_version != geometry.vehicle_model_version
            || speedProfile->configuration_id.empty()
            || speedProfile->configuration_id != geometry.vehicle_configuration_id
            || speedProfile->dynamics_source.empty()
            || (speedProfile->speed_reference != "TAS"
                && speedProfile->speed_reference != "AIR_RELATIVE")
            || speedProfile->segments.size() != geometry.segments.size()) {
            return fail("MT01_ROUTE_CANDIDATE_SPEED_PROFILE_INVALID: a feasible, identified profile covering every geometry segment is required.");
        }
    }

    route.segments.reserve(geometry.segments.size());
    for (std::size_t i = 0; i < geometry.segments.size(); ++i) {
        const auto& source = geometry.segments[i];
        if (source.from_waypoint_index != i || source.to_waypoint_index != i + 1
            || source.segment_id.empty() || !std::isfinite(source.distance_m)
            || source.distance_m <= 0.0 || !std::isfinite(source.nominal_speed_mps)
            || source.nominal_speed_mps <= 0.0
            || !std::isfinite(source.bank_angle_rad) || source.bank_angle_rad < 0.0
            || !std::isfinite(source.lateral_acceleration_mps2)
            || source.lateral_acceleration_mps2 < 0.0) {
            return fail("MT01_ROUTE_CANDIDATE_SEGMENT_INVALID: every segment must be finite, ordered and carry explicit dynamics.");
        }
        RouteSegment segment;
        segment.segment_id = source.segment_id;
        segment.from_waypoint_id = route.waypoints[i].waypoint_id;
        segment.to_waypoint_id = route.waypoints[i + 1].waypoint_id;
        segment.distance_m = source.distance_m;
        segment.nominal_speed_mps = source.nominal_speed_mps;
        segment.bank_angle_rad = source.bank_angle_rad;
        segment.lateral_acceleration_mps2 = source.lateral_acceleration_mps2;
        route.segments.push_back(std::move(segment));
    }

    if (speedProfile) {
        RouteSpeedProfile boundProfile;
        boundProfile.vehicle_model_id = speedProfile->vehicle_model_id;
        boundProfile.vehicle_model_version = speedProfile->vehicle_model_version;
        boundProfile.configuration_id = speedProfile->configuration_id;
        boundProfile.speed_reference = speedProfile->speed_reference;
        boundProfile.dynamics_source = speedProfile->dynamics_source;
        boundProfile.segments.reserve(speedProfile->segments.size());
        double previousExitSpeed = 0.0;
        for (std::size_t i = 0; i < speedProfile->segments.size(); ++i) {
            const auto& profileSegment = speedProfile->segments[i];
            const auto& routeSegment = route.segments[i];
            const auto finitePositive = [](double value) {
                return std::isfinite(value) && value > 0.0;
            };
            const double distanceTolerance = std::max(0.5, routeSegment.distance_m * 0.01);
            const double phaseDistance = profileSegment.acceleration_distance_m
                + profileSegment.deceleration_distance_m + profileSegment.cruise_distance_m;
            if (profileSegment.segment_id != routeSegment.segment_id
                || !finitePositive(profileSegment.distance_m)
                || std::abs(profileSegment.distance_m - routeSegment.distance_m) > distanceTolerance
                || !finitePositive(profileSegment.entry_speed_mps)
                || !finitePositive(profileSegment.exit_speed_mps)
                || !finitePositive(profileSegment.maximum_reachable_speed_mps)
                || profileSegment.maximum_reachable_speed_mps
                    > routeSegment.nominal_speed_mps + 1e-6
                || profileSegment.entry_speed_mps > routeSegment.nominal_speed_mps + 1e-6
                || profileSegment.exit_speed_mps > routeSegment.nominal_speed_mps + 1e-6
                || profileSegment.entry_speed_mps
                    < geometry.segments[i].minimum_speed_mps - 1e-6
                || profileSegment.exit_speed_mps
                    < geometry.segments[i].minimum_speed_mps - 1e-6
                || profileSegment.maximum_reachable_speed_mps
                    < geometry.segments[i].minimum_speed_mps - 1e-6
                || !finitePositive(profileSegment.maximum_acceleration_mps2)
                || !finitePositive(profileSegment.maximum_deceleration_mps2)
                || !std::isfinite(profileSegment.acceleration_distance_m)
                || !std::isfinite(profileSegment.deceleration_distance_m)
                || !std::isfinite(profileSegment.cruise_distance_m)
                || profileSegment.acceleration_distance_m < 0.0
                || profileSegment.deceleration_distance_m < 0.0
                || profileSegment.cruise_distance_m < 0.0
                || std::abs(phaseDistance - routeSegment.distance_m) > distanceTolerance
                || profileSegment.maximum_reachable_speed_mps
                    + 1e-9 < std::max(profileSegment.entry_speed_mps,
                                      profileSegment.exit_speed_mps)
                || (i > 0 && std::abs(profileSegment.entry_speed_mps
                                      - previousExitSpeed) > 1e-6)) {
                return fail("MT01_ROUTE_CANDIDATE_SPEED_PROFILE_BINDING_INVALID: profile segment order, geometry distance, kinematics or boundary continuity does not match the canonical route.");
            }
            boundProfile.segments.push_back({
                profileSegment.segment_id, profileSegment.distance_m,
                profileSegment.entry_speed_mps, profileSegment.exit_speed_mps,
                profileSegment.maximum_reachable_speed_mps,
                profileSegment.acceleration_distance_m,
                profileSegment.deceleration_distance_m,
                profileSegment.cruise_distance_m,
                profileSegment.maximum_acceleration_mps2,
                profileSegment.maximum_deceleration_mps2,
                geometry.segments[i].minimum_speed_mps,
                routeSegment.nominal_speed_mps,
                geometry.segments[i].minimum_speed_mps,
                routeSegment.nominal_speed_mps
            });
            previousExitSpeed = profileSegment.exit_speed_mps;
        }
        route.speed_profile = std::move(boundProfile);
    }

    result.structural_validation = RouteConstraintValidator::validate(route);
    if (result.structural_validation.status != RouteValidationStatus::Allowed) {
        for (const auto& finding : result.structural_validation.findings)
            result.diagnostics.push_back(finding.detail);
        return fail("MT01_ROUTE_CANDIDATE_STRUCTURAL_VALIDATION_FAILED: canonical route structure was rejected.");
    }

    result.available = true;
    result.diagnostics.push_back(
        "MT01_STRUCTURAL_ROUTE_CANDIDATE: canonical waypoint/segment chain and AGL target passed structural validation.");
    result.diagnostics.push_back(
        "MT01_TRAJECTORY_GEOMETRY_NOT_COMPLETE: terrain, wind-ground-track, NOTAM/airspace, obstacle and independent release checks remain mandatory.");
    return result;
}

} // namespace bluesky::planning
