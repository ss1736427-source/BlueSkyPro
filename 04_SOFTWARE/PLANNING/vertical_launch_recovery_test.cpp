#include "vertical_launch_recovery.hpp"
#include <cassert>
using namespace bluesky::planning;
int main() {
    VerticalLaunchRecoveryInput i;
    i.point={59.0,30.0};
    i.terrain_elevation_m=20.0;
    i.minimum_clearance_m=30.0;
    i.regulatory_minimum_altitude_m=60.0;
    i.vehicle_minimum_transition_altitude_m=50.0;
    i.vehicle_max_altitude_m=150.0;
    i.obstacles.push_back({"TOWER-1",80.0});
    auto r=VerticalLaunchRecoveryPlanner::calculate(i,"VLR-1");
    assert(r.status==VerticalProfileStatus::Valid);
    assert(r.safe_transition_altitude_m==110.0);
    i.vehicle_max_altitude_m=100.0;
    auto blocked=VerticalLaunchRecoveryPlanner::calculate(i,"VLR-1");
    assert(blocked.status==VerticalProfileStatus::Infeasible);
    assert(!blocked.blocking_obstacle_ids.empty());
    return 0;
}
