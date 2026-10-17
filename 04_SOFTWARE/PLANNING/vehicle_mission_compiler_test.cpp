#include "vehicle_mission_compiler.hpp"
#include <cassert>
#include <cmath>
#include <limits>

using namespace bluesky::planning;

int main() {
    FlightProfile p;
    p.mission_id = "M";
    p.mission_version = "1";
    p.profile_id = "FP";
    p.profile_version = "1.0.0";
    p.source_candidate_id = "C";
    p.route_version = "R1";
    p.calculation_input_version = "ENV1";
    p.dependency_identity = "D";
    p.valid = true;
    p.points = {
        {{0.0, 0.0}, 50.0, 0.0, true},
        {{0.0, 0.001}, 60.0, 10.0, true}
    };

    VehicleCompilationInput i;
    i.uav_id = "U1";
    i.configuration_version = "CFG1";
    i.performance_version = "PERF1";
    i.maximum_altitude_m = 120.0;
    i.minimum_speed_mps = 5.0;
    i.maximum_speed_mps = 20.0;
    i.cruise_speed_mps = 12.0;

    auto r = VehicleMissionCompiler::compile(p, i);
    assert(r.status == VehicleCompilationStatus::Compiled);
    assert(r.points.size() == 2);
    assert(r.route_version == "R1");
    assert(r.calculation_input_version == "ENV1");

    auto invalid_profile = p;
    invalid_profile.valid = false;
    r = VehicleMissionCompiler::compile(invalid_profile, i);
    assert(r.status == VehicleCompilationStatus::Infeasible);
    assert(!r.findings.empty());
    assert(r.findings[0].code == VehicleCompilationFindingCode::InvalidFlightProfile);

    auto empty_profile = p;
    empty_profile.points.clear();
    r = VehicleMissionCompiler::compile(empty_profile, i);
    assert(r.status == VehicleCompilationStatus::Infeasible);

    auto invalid_timing = p;
    invalid_timing.points[1].cumulative_time_s = std::numeric_limits<double>::quiet_NaN();
    r = VehicleMissionCompiler::compile(invalid_timing, i);
    assert(r.status == VehicleCompilationStatus::Infeasible);

    auto missing_vehicle_identity = i;
    missing_vehicle_identity.configuration_version.clear();
    r = VehicleMissionCompiler::compile(p, missing_vehicle_identity);
    assert(r.status == VehicleCompilationStatus::Infeasible);
    assert(r.findings[0].code == VehicleCompilationFindingCode::InvalidVehicleIdentity);

    auto invalid_limits = i;
    invalid_limits.maximum_speed_mps = std::numeric_limits<double>::quiet_NaN();
    r = VehicleMissionCompiler::compile(p, invalid_limits);
    assert(r.status == VehicleCompilationStatus::Infeasible);
    assert(r.findings[0].code == VehicleCompilationFindingCode::SpeedLimitInvalid);

    i.maximum_altitude_m = 40.0;
    r = VehicleMissionCompiler::compile(p, i);
    assert(r.status == VehicleCompilationStatus::Infeasible);
    assert(r.findings[0].code == VehicleCompilationFindingCode::AltitudeLimitExceeded);
    return 0;
}
