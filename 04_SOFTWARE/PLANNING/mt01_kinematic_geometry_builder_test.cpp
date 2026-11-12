#include "mt01_kinematic_geometry_builder.hpp"

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
    assert(result.diagnostics.size() >= 2);

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
