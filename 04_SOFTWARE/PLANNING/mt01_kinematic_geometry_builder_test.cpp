#include <algorithm>
#include "mt01_kinematic_geometry_builder.hpp"
#include "mt01_route_candidate_builder.hpp"
#include "model/flight_safety_constraints.hpp"

#include <cassert>
#include <cmath>

using namespace bluesky::planning;

int main()
{
    CoveragePlanResult coverage;
    coverage.valid = true;
    coverage.status = "CANDIDATE_2D_COVERAGE_ONLY";
    // Two alternating straight coverage legs, approximately 100 m apart.
    coverage.waypoints = {
        {0.0, 0.0}, {0.0, 0.002},
        {0.001, 0.002}, {0.001, 0.0}
    };
    coverage.capture_points = {
        {{0.0, 0.0}, 0, 0.0},
        {{0.0, 0.001}, 0, 111.195},
        {{0.0, 0.002}, 0, 222.390},
        {{0.001, 0.002}, 1, 0.0},
        {{0.001, 0.001}, 1, 111.195},
        {{0.001, 0.0}, 1, 222.390}
    };

    VehicleMotionModel vehicle;
    vehicle.model_id = "TEST-FIXED-WING";
    vehicle.model_version = "fixture-1";
    vehicle.configuration_id = "TEST-CONFIG-1";
    vehicle.motion_class = MotionClass::FixedWing;
    vehicle.evidence = MotionEvidence::FlightTestValidated;
    vehicle.reference_speed_mps = 10.0;
    vehicle.minimum_speed_mps = 5.0;
    vehicle.maximum_speed_mps = 20.0;
    vehicle.bank_angle_rad = 30.0 * 3.14159265358979323846 / 180.0;
    vehicle.speed_reference = "TAS";
    vehicle.source_reference = "synthetic test fixture only";

    const auto result = Mt01KinematicGeometryBuilder::build(
        coverage, vehicle, "TEST-FIXED-WING", "TEST-CONFIG-1",
        10.0, {});
    assert(result.feasible);
    assert(result.connector_count == 1);
    assert(result.minimum_turn_radius_m > 0.0);
    assert(result.waypoints.size() > coverage.waypoints.size());
    assert(result.segments.size() + 1 == result.waypoints.size());
    assert(result.capture_point_waypoint_indices.size() == coverage.capture_points.size());
    for (std::size_t i = 0; i < result.capture_point_waypoint_indices.size(); ++i) {
        const auto waypointIndex = result.capture_point_waypoint_indices[i];
        assert(waypointIndex < result.waypoints.size());
        const auto& mapped = result.waypoints[waypointIndex];
        assert(std::abs(mapped.latitude_deg - coverage.capture_points[i].position.latitude_deg) < 1e-9);
        assert(std::abs(mapped.longitude_deg - coverage.capture_points[i].position.longitude_deg) < 1e-9);
    }

    const auto requestedTurnSpeed =
        VehicleMotionModelEvaluator::recommendTurnSpeedForRadius(vehicle, 15.0);
    assert(requestedTurnSpeed.valid);
    const auto slowedConnectorGeometry = Mt01KinematicGeometryBuilder::build(
        coverage, vehicle, "TEST-FIXED-WING", "TEST-CONFIG-1",
        10.0, {}, 2.0, requestedTurnSpeed.selected_speed_mps);
    assert(slowedConnectorGeometry.feasible);
    assert(std::abs(slowedConnectorGeometry.minimum_turn_radius_m - 15.0) < 1e-6);
    bool sawSlowedConnector = false;
    bool sawCruiseCoverage = false;
    for (const auto& segment : slowedConnectorGeometry.segments) {
        if (segment.type == Mt01KinematicSegmentType::ConnectorTurn
            || segment.type == Mt01KinematicSegmentType::ConnectorStraight) {
            sawSlowedConnector = true;
            assert(std::abs(segment.nominal_speed_mps
                            - requestedTurnSpeed.selected_speed_mps) < 1e-9);
        } else if (segment.type == Mt01KinematicSegmentType::Coverage) {
            sawCruiseCoverage = true;
            assert(std::abs(segment.nominal_speed_mps - 10.0) < 1e-9);
        }
    }
    assert(sawSlowedConnector && sawCruiseCoverage);
    const auto belowMinimumConnector = Mt01KinematicGeometryBuilder::build(
        coverage, vehicle, "TEST-FIXED-WING", "TEST-CONFIG-1",
        10.0, {}, 2.0, 4.9);
    assert(!belowMinimumConnector.feasible);
    assert(belowMinimumConnector.diagnostics.front().find(
        "CONNECTOR_SPEED_OUTSIDE_ENVELOPE") != std::string::npos);

    assert(result.diagnostics.size() >= 2);
    bool sawCoverage = false;
    bool sawTurn = false;
    std::vector<FlightSafetySegmentState> segmentStates;
    for (std::size_t i = 0; i < result.segments.size(); ++i) {
        const auto& segment = result.segments[i];
        assert(segment.from_waypoint_index == i);
        assert(segment.to_waypoint_index == i + 1);
        assert(segment.distance_m > 0.0);
        assert(segment.nominal_speed_mps == 10.0);
        sawCoverage = sawCoverage || segment.type == Mt01KinematicSegmentType::Coverage;
        sawTurn = sawTurn || segment.type == Mt01KinematicSegmentType::ConnectorTurn;
        segmentStates.push_back({
            segment.segment_id,
            segment.nominal_speed_mps,
            segment.bank_angle_rad,
            segment.lateral_acceleration_mps2
        });
    }
    assert(sawCoverage);
    assert(sawTurn);

    FlightSafetyEnvelope envelope;
    envelope.vehicle_model_id = "TEST-FIXED-WING";
    envelope.model_version = "fixture-1";
    envelope.configuration_id = "TEST-CONFIG-1";
    envelope.source_reference = "controlled test envelope";
    envelope.speed_reference = "TAS";
    envelope.minimum_speed_basis =
        "CONFIGURATION_SPECIFIC_STALL_SPEED_WITH_BANK_AND_MARGIN";
    envelope.evidence = FlightSafetyEvidence::FlightTestValidated;
    envelope.motion_class = MotionClass::FixedWing;
    envelope.minimum_speed_mps = 5.0;
    envelope.maximum_speed_mps = 20.0;
    envelope.maximum_bank_angle_rad = vehicle.bank_angle_rad;
    envelope.maximum_lateral_acceleration_mps2 = 6.0;
    const auto dynamicsCheck =
        FlightSafetyConstraintValidator::validate(envelope, segmentStates);
    assert(dynamicsCheck.status == FlightSafetyStatus::Allowed);
    // The connector deliberately uses the validated maximum bank angle; this
    // is allowed but must be disclosed to the operator.
    assert(dynamicsCheck.maximum_regime_warning_required);
    assert(!dynamicsCheck.maximum_regime_uses.empty());
    assert(dynamicsCheck.operator_warning.find("максимально допустимых режимов БВС") !=
           std::string::npos);

    coverage.metrics.calculated_altitude_agl_m = 100.0;
    Mt01RouteCandidateMetadata routeMetadata;
    routeMetadata.route_id = "MT01-ROUTE-TEST";
    routeMetadata.route_version = "1";
    routeMetadata.generator_id = "MT01-KINEMATIC-GENERATOR";
    routeMetadata.generator_version = "1";
    routeMetadata.calculation_input_version = "INPUT-TEST-1";
    routeMetadata.constraints = {50.0, 150.0, 30.0, true};
    SpeedProfileInput profileInput;
    profileInput.vehicle_model_id = vehicle.model_id;
    profileInput.vehicle_model_version = vehicle.model_version;
    profileInput.configuration_id = vehicle.configuration_id;
    profileInput.speed_reference = "TAS";
    profileInput.longitudinal_dynamics_validated = true;
    profileInput.longitudinal_dynamics_source = "controlled-test-fixture";
    profileInput.maximum_acceleration_mps2 = 2.0;
    profileInput.maximum_deceleration_mps2 = 3.0;
    profileInput.initial_speed_mps = 10.0;
    profileInput.final_speed_mps = 10.0;
    for (const auto& segment : result.segments)
        profileInput.segments.push_back({segment.segment_id, segment.distance_m, 5.0, 10.0});
    const auto speedProfile = VariableSpeedProfileSolver::calculate(profileInput);
    assert(speedProfile.status == SpeedProfileStatus::Feasible);
    const auto routeCandidate = Mt01RouteCandidateBuilder::build(
        result, coverage, "MT01-MISSION-TEST", "1", routeMetadata, &speedProfile);
    assert(routeCandidate.available);
    assert(routeCandidate.route.speed_profile.has_value());
    assert(routeCandidate.route.speed_profile->segments.size() == result.segments.size());
    assert(routeCandidate.structural_validation.status == RouteValidationStatus::Allowed);
    assert(routeCandidate.route.waypoints.size() == result.waypoints.size());
    assert(routeCandidate.route.segments.size() == result.segments.size());
    for (std::size_t i = 0; i < result.segments.size(); ++i) {
        assert(routeCandidate.route.speed_profile->segments[i].segment_id ==
               routeCandidate.route.segments[i].segment_id);
        assert(std::abs(routeCandidate.route.speed_profile->segments[i].distance_m -
                        routeCandidate.route.segments[i].distance_m) <=
               std::max(0.5, routeCandidate.route.segments[i].distance_m * 0.01));
    }

    SpeedProfileInput slowedProfileInput;
    slowedProfileInput.vehicle_model_id = vehicle.model_id;
    slowedProfileInput.vehicle_model_version = vehicle.model_version;
    slowedProfileInput.configuration_id = vehicle.configuration_id;
    slowedProfileInput.speed_reference = "TAS";
    slowedProfileInput.longitudinal_dynamics_validated = true;
    slowedProfileInput.longitudinal_dynamics_source = "controlled-test-fixture";
    slowedProfileInput.maximum_acceleration_mps2 = 2.0;
    slowedProfileInput.maximum_deceleration_mps2 = 3.0;
    slowedProfileInput.initial_speed_mps = 10.0;
    slowedProfileInput.final_speed_mps = 10.0;
    for (const auto& segment : slowedConnectorGeometry.segments) {
        slowedProfileInput.segments.push_back({
            segment.segment_id, segment.distance_m, 5.0,
            segment.nominal_speed_mps
        });
    }
    const auto slowedProfile =
        VariableSpeedProfileSolver::calculate(slowedProfileInput);
    assert(slowedProfile.status == SpeedProfileStatus::Feasible);
    const auto slowedRoute = Mt01RouteCandidateBuilder::build(
        slowedConnectorGeometry, coverage, "MT01-MISSION-TEST", "1",
        routeMetadata, &slowedProfile);
    assert(slowedRoute.available);
    assert(slowedRoute.route.speed_profile.has_value());

    auto overCapProfile = slowedProfile;
    for (std::size_t i = 0; i < slowedConnectorGeometry.segments.size(); ++i) {
        if (slowedConnectorGeometry.segments[i].type
            == Mt01KinematicSegmentType::ConnectorTurn) {
            overCapProfile.segments[i].maximum_reachable_speed_mps += 0.5;
            break;
        }
    }
    const auto overCapRoute = Mt01RouteCandidateBuilder::build(
        slowedConnectorGeometry, coverage, "MT01-MISSION-TEST", "1",
        routeMetadata, &overCapProfile);
    assert(!overCapRoute.available);
    assert(overCapRoute.diagnostics.front().find("SPEED_PROFILE") !=
           std::string::npos);

    auto incompleteProfile = speedProfile;
    incompleteProfile.segments.pop_back();
    const auto rejectedProfile = Mt01RouteCandidateBuilder::build(
        result, coverage, "MT01-MISSION-TEST", "1", routeMetadata, &incompleteProfile);
    assert(!rejectedProfile.available);
    assert(rejectedProfile.diagnostics.front().find("SPEED_PROFILE") != std::string::npos);
    assert(!routeCandidate.route.trajectory_geometry_complete);
    const auto routeDynamics = FlightSafetyConstraintValidator::validateRoute(
        envelope, routeCandidate.route);
    assert(routeDynamics.status == FlightSafetyStatus::Allowed);
    assert(routeDynamics.maximum_regime_warning_required);

    auto missingRouteLineage = routeMetadata;
    missingRouteLineage.calculation_input_version.clear();
    const auto blockedRoute = Mt01RouteCandidateBuilder::build(
        result, coverage, "MT01-MISSION-TEST", "1", missingRouteLineage);
    assert(!blockedRoute.available);
    assert(!blockedRoute.diagnostics.empty());

    auto insufficientEnvelope = envelope;
    insufficientEnvelope.maximum_bank_angle_rad = vehicle.bank_angle_rad * 0.8;
    const auto rejectedDynamics =
        FlightSafetyConstraintValidator::validate(insufficientEnvelope, segmentStates);
    assert(rejectedDynamics.status == FlightSafetyStatus::Rejected);

    auto wrongConfiguration = vehicle;
    wrongConfiguration.configuration_id = "OTHER-CONFIG";
    const auto mismatch = Mt01KinematicGeometryBuilder::build(
        coverage, wrongConfiguration, "TEST-FIXED-WING", "TEST-CONFIG-1",
        10.0, {});
    assert(!mismatch.feasible);
    assert(mismatch.diagnostics.front().find("BINDING_MISMATCH") != std::string::npos);

    auto unvalidated = vehicle;
    unvalidated.evidence = MotionEvidence::DerivedScenario;
    const auto noEvidence = Mt01KinematicGeometryBuilder::build(
        coverage, unvalidated, "TEST-FIXED-WING", "TEST-CONFIG-1",
        10.0, {});
    assert(!noEvidence.feasible);
    assert(noEvidence.diagnostics.front().find("NOT_VALIDATED") != std::string::npos);

    auto multirotor = vehicle;
    multirotor.model_id = "TEST-MULTIROTOR";
    multirotor.motion_class = MotionClass::Multirotor;
    multirotor.speed_reference = "AIR_RELATIVE";
    multirotor.lateral_acceleration_mps2 = 2.0;
    multirotor.bank_angle_rad = 0.0; // Bank is not a multirotor turn constraint.
    const auto multirotorGeometry = Mt01KinematicGeometryBuilder::build(
        coverage, multirotor, "TEST-MULTIROTOR", "TEST-CONFIG-1",
        10.0, {});
    assert(multirotorGeometry.feasible);
    assert(multirotorGeometry.minimum_turn_radius_m > 0.0);
    bool sawMultirotorTurn = false;
    for (const auto& segment : multirotorGeometry.segments) {
        if (segment.type == Mt01KinematicSegmentType::ConnectorTurn) {
            sawMultirotorTurn = true;
            assert(segment.bank_angle_rad == 0.0);
            assert(std::abs(segment.lateral_acceleration_mps2 - 2.0) < 1e-6);
        }
    }
    assert(sawMultirotorTurn);

    const std::vector<std::vector<GeoPoint>> blockingArea{{
        {-0.01, -0.01}, {-0.01, 0.01}, {0.01, 0.01}, {0.01, -0.01}
    }};
    const auto blocked = Mt01KinematicGeometryBuilder::build(
        coverage, vehicle, "TEST-FIXED-WING", "TEST-CONFIG-1",
        10.0, blockingArea);
    assert(!blocked.feasible);
    assert(blocked.diagnostics.front().find("TURN_CONNECTOR_BLOCKED") != std::string::npos);

    return 0;
}
