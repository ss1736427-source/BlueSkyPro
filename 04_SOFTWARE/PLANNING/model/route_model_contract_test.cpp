#include "route_model.hpp"

#include <cassert>

using namespace bluesky::planning;

int main() {
    Route route;
    route.lineage = {
        "ROUTE-001", "1", "MISSION-001", "1",
        "deterministic-route-generator", "1", "INPUT-001"
    };
    route.constraints = {80.0, 120.0, 30.0, true};
    route.environment = {"DEM-01", "AIRSPACE-01", "WX-01", "WIND-01"};

    route.waypoints.push_back({"WP-001", {60.1699, 24.9384}, 100.0, true});
    route.waypoints.push_back({"WP-002", {60.1705, 24.9500}, 100.0, false});
    route.segments.push_back({"SEG-001", "WP-001", "WP-002", 700.0, 20.0});

    assert(route.lineage.route_id == "ROUTE-001");
    assert(route.lineage.mission_id == "MISSION-001");
    assert(route.waypoints.size() == 2);
    assert(route.waypoints.front().mandatory);
    assert(route.segments.size() == 1);
    assert(route.constraints.avoid_restricted_areas);
    assert(route.environment.wind_snapshot_id == "WIND-01");
    return 0;
}
