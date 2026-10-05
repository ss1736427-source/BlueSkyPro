#include "multi_uav_conflict_resolution.hpp"
#include <cassert>
#include <cmath>
#include <iostream>

using namespace bluesky::planning;

namespace {
TrajectoryResult makeTrajectory(const std::string& id, double x0, double x1, double y0, double y1) {
    TrajectoryResult t;
    t.status = TrajectoryStatus::Feasible;
    t.uav_id = id;
    t.route_id = "route-"+id;
    t.route_version = "1";
    t.dependency_identity = "dep-"+id;
    t.points = {
        {"P0", {0.0, x0}, 100.0, 0.0, 0.0},
        {"P1", {0.0, x1}, 100.0, 10.0, 1.0}
    };
    return t;
}
}

int main() {
    auto a = makeTrajectory("A", 0.0, 0.002, 0.0, 0.0);
    auto b = makeTrajectory("B", 0.001, 0.001, -0.001, 0.001);
    // Construct a same-altitude crossing around t=5 s.
    a.points[0].position.latitude_deg = -0.001;
    a.points[1].position.latitude_deg = 0.001;
    b.points[0].position.longitude_deg = -0.001;
    b.points[1].position.longitude_deg = 0.001;

    const std::vector<MultiUavSeparationInput> separation = {
        {"A", 50.0, 1.0, 0.0}, {"B", 50.0, 1.0, 0.0}
    };
    const std::vector<MultiUavResolutionInput> limits = {
        {"A", 1.0, 90.0, 120.0}, {"B", 1.0, 90.0, 120.0}
    };
    const auto r = MultiUavConflictResolver::resolve({a,b}, separation, limits, "RES-1");
    assert(r.status == MultiUavResolutionStatus::Resolved);
    assert(r.trajectories.size() == 2);
    assert(r.dependency_identity.find("START_DELAY_WINDOW:5") != std::string::npos);
    assert((r.findings[0].code == MultiUavResolutionFindingCode::StartDelayApplied) ||
           (r.trajectories[0].dependency_identity.find("VERTICAL_RESOLUTION") != std::string::npos));
    assert(r.trajectories[0].points.front().altitude_m == 100.0);
    assert(r.trajectories[1].points.back().altitude_m == 100.0);

    const auto clear = MultiUavConflictResolver::resolve(
        {a, makeTrajectory("C", 0.0, 0.002, 0.01, 0.012)}, separation, limits, "RES-2");
    assert(clear.status == MultiUavResolutionStatus::Resolved);

    const std::vector<MultiUavResolutionInput> blocked = {
        {"A", 1.0, 100.0, 100.0}, {"B", 1.0, 100.0, 100.0}
    };
    const std::vector<MultiUavSeparationInput> blockedSeparation = {
        {"A", 50.0, 1.0, 10.0}, {"B", 50.0, 1.0, 10.0}
    };
    const auto blockedResult = MultiUavConflictResolver::resolve({a,b}, blockedSeparation, blocked, "RES-3");
    assert(blockedResult.status == MultiUavResolutionStatus::Infeasible);
    std::cout << "multi_uav_conflict_resolution_test: OK\n";
    return 0;
}
