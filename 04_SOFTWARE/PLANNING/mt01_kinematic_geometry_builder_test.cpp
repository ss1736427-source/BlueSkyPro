#include "mt01_kinematic_geometry_builder.hpp"
#include "mt01_task_execution_event_adapter.hpp"
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
    const auto routeCandidate = Mt01RouteCandidateBuilder::build(
        result, coverage, "MT01-MISSION-TEST", "1", routeMetadata);
    assert(routeCandidate.available);
    assert(routeCandidate.structural_validation.status == RouteValidationStatus::Allowed);
    assert(routeCandidate.route.waypoints.size() == result.waypoints.size());
    assert(routeCandidate.route.segments.size() == result.segments.size());
    assert(!routeCandidate.route.trajectory_geometry_complete);
    std::vector<Mt01TaskExecutionEvent> events;
    for (std::size_t i=0;i<coverage.capture_points.size();++i) {
        Mt01TaskExecutionEvent e;
        e.capture_point_index=i; e.coverage_segment_index=coverage.capture_points[i].coverage_segment_index;
        e.route_waypoint_index=result.capture_point_waypoint_indices[i]; e.duration_s=2.0; e.energy_wh=0.25;
        e.mission_id=routeCandidate.route.lineage.mission_id; e.mission_version=routeCandidate.route.lineage.mission_version;
        e.route_id=routeCandidate.route.lineage.route_id; e.route_version=routeCandidate.route.lineage.route_version;
        e.vehicle_id="TEST-FIXED-WING"; e.vehicle_configuration_version="TEST-CONFIG-V1";
        e.equipment_id="TEST-CAMERA"; e.equipment_profile_id="TEST-CAMERA-PROFILE";
        e.calibration_reference="TEST-CALIBRATION"; e.source_reference="controlled per-capture estimate";
        e.source_version="fixture-1"; e.method_version="capture-cost-v1"; events.push_back(e);
    }
    auto eventRoute=routeCandidate.route;
    const auto eventResult=Mt01TaskExecutionEventAdapter::apply(eventRoute,coverage,result,events,
        "TEST-FIXED-WING","TEST-CONFIG-V1","TEST-CAMERA","TEST-CAMERA-PROFILE","TEST-CALIBRATION");
    assert(eventResult.valid);
    assert(std::abs(eventResult.total_duration_s-2.0*coverage.capture_points.size())<1e-9);
    assert(std::abs(eventResult.total_energy_wh-0.25*coverage.capture_points.size())<1e-9);
    auto missingEvents=events; missingEvents.pop_back(); auto missingRoute=routeCandidate.route;
    assert(!Mt01TaskExecutionEventAdapter::apply(missingRoute,coverage,result,missingEvents,
        "TEST-FIXED-WING","TEST-CONFIG-V1","TEST-CAMERA","TEST-CAMERA-PROFILE","TEST-CALIBRATION").valid);
    auto duplicateEvents=events; duplicateEvents.back().capture_point_index=duplicateEvents.front().capture_point_index;
    auto duplicateRoute=routeCandidate.route;
    assert(!Mt01TaskExecutionEventAdapter::apply(duplicateRoute,coverage,result,duplicateEvents,
        "TEST-FIXED-WING","TEST-CONFIG-V1","TEST-CAMERA","TEST-CAMERA-PROFILE","TEST-CALIBRATION").valid);
    auto wrongLineage=events; wrongLineage.front().vehicle_configuration_version="OTHER-CONFIG";
    auto wrongRoute=routeCandidate.route;
    assert(!Mt01TaskExecutionEventAdapter::apply(wrongRoute,coverage,result,wrongLineage,
        "TEST-FIXED-WING","TEST-CONFIG-V1","TEST-CAMERA","TEST-CAMERA-PROFILE","TEST-CALIBRATION").valid);
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
    multirotor.motion_class = MotionClass::Multirotor;
    multirotor.speed_reference = "AIR_RELATIVE";
    multirotor.lateral_acceleration_mps2 = 2.0;
    const auto unsupported = Mt01KinematicGeometryBuilder::build(
        coverage, multirotor, "TEST-FIXED-WING", "TEST-CONFIG-1",
        10.0, {});
    assert(!unsupported.feasible);
    assert(unsupported.diagnostics.front().find("supports fixed-wing") != std::string::npos);

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
