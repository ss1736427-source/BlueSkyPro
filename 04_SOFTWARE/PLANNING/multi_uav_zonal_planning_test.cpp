#include "multi_uav_zonal_planning.hpp"
#include <cassert>
#include <iostream>

using namespace bluesky::planning;

int main() {
    MissionOperatingArea area{
        "AREA-1", "v1",
        {{50.0, 10.0}, {51.0, 10.0}, {51.0, 13.0}, {50.0, 13.0}}
    };

    const auto result = MultiUavZonalPlanner::partition(
        area, {"UAV-B", "UAV-A", "UAV-C"}, "zonal-1");

    assert(result.feasible);
    assert(result.zones.size() == 3);
    assert(result.assignments.size() == 3);
    assert(result.assignments[0].uav_id == "UAV-A");
    assert(result.assignments[1].uav_id == "UAV-B");
    assert(result.assignments[2].uav_id == "UAV-C");
    assert(result.zones[0].boundary[0].longitude_deg == 10.0);
    assert(result.zones[1].boundary[0].longitude_deg > result.zones[0].boundary[2].longitude_deg - 1.0);
    assert(!result.dependency_identity.empty());

    const auto invalid = MultiUavZonalPlanner::partition(
        area, {"UAV-A", "UAV-A"}, "zonal-1");
    assert(!invalid.feasible);

    const auto degenerate = MultiUavZonalPlanner::partition(
        MissionOperatingArea{"AREA-2", "v1", {{50.0, 10.0}, {50.0, 10.0}, {50.0, 10.0}}},
        {"UAV-A"}, "zonal-1");
    assert(!degenerate.feasible);

    std::cout << "multi_uav_zonal_planning_test: PASS\n";
}
