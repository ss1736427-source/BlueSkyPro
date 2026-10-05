#include "wind_performance_trajectory.hpp"
#include <cassert>
#include <cmath>

using namespace bluesky::planning;

int main(){
    Route route;
    route.lineage.route_id="R1";
    route.lineage.route_version="1";
    route.lineage.calculation_input_version="ENV1";
    route.waypoints={
        {"A",{0.0,0.0},100.0,true},
        {"B",{0.0,0.001},100.0,true}
    };
    route.segments={{"S1","A","B",111.32,10.0}};

    TrajectoryPerformanceInput in;
    in.uav_id="U1";
    in.configuration_version="CFG1";
    in.performance_version="PERF1";
    in.wind_snapshot_id="W1";
    in.wind_snapshot_version="1";
    in.cruise_airspeed_mps=20.0;
    in.climb_rate_mps=2.0;
    in.descent_rate_mps=2.0;
    in.wind_tolerance_mps=15.0;
    in.energy_per_horizontal_meter_wh=0.01;
    in.usable_energy_wh=100.0;
    in.reserve_requirement_wh=10.0;
    in.wind_samples={{"S1",0.0,5.0,0.0}};

    const auto r=WindPerformanceTrajectory::calculate(route,in,"CALC1");
    assert(r.status==TrajectoryStatus::Feasible);
    assert(r.segments.size()==1);
    assert(r.points.size()==2);
    assert(r.segments[0].ground_speed_mps>20.0);
    assert(r.segments[0].traversal_time_s>5.0);
    assert(r.total_energy_wh>1.0);
    assert(r.remaining_energy_wh<100.0);
    assert(!r.dependency_identity.empty());

    auto missing=in; missing.wind_samples.clear();
    assert(WindPerformanceTrajectory::calculate(route,missing,"CALC1").status==TrajectoryStatus::Infeasible);

    auto excessive=in; excessive.wind_samples[0].north_mps=20.0;
    assert(WindPerformanceTrajectory::calculate(route,excessive,"CALC1").status==TrajectoryStatus::Infeasible);

    auto low_energy=in; low_energy.usable_energy_wh=1.0;
    assert(WindPerformanceTrajectory::calculate(route,low_energy,"CALC1").status==TrajectoryStatus::Infeasible);

    Route vertical=route;
    vertical.waypoints[1].position=vertical.waypoints[0].position;
    vertical.waypoints[1].altitude_m=120.0;
    vertical.segments[0].distance_m=0.0;
    auto vr=WindPerformanceTrajectory::calculate(vertical,in,"CALC1");
    assert(vr.status==TrajectoryStatus::Feasible);
    assert(vr.segments[0].ground_speed_mps==0.0);
    assert(vr.segments[0].traversal_time_s>9.9);

    return 0;
}
