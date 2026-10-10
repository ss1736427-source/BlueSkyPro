#include "vehicle_mission_compiler.hpp"
#include <cassert>
using namespace bluesky::planning;
int main() {
    FlightProfile p; p.mission_id="M"; p.mission_version="1"; p.profile_id="FP"; p.dependency_identity="D";
    p.points.push_back({{0,0},50,0,true});
    VehicleCompilationInput i; i.uav_id="U1"; i.configuration_version="CFG1"; i.performance_version="PERF1";
    i.maximum_altitude_m=120; i.minimum_speed_mps=5; i.maximum_speed_mps=20; i.cruise_speed_mps=12;
    auto r=VehicleMissionCompiler::compile(p,i);
    assert(r.status==VehicleCompilationStatus::Compiled);
    assert(r.points.size()==1);
    i.maximum_altitude_m=40;
    r=VehicleMissionCompiler::compile(p,i);
    assert(r.status==VehicleCompilationStatus::Infeasible);
    return 0;
}
