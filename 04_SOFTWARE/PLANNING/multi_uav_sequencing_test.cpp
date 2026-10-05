#include "multi_uav_sequencing.hpp"
#include <cassert>

using namespace bluesky::planning;

namespace {
TrajectoryResult makeTrajectory(const std::string& id, double start, double end) {
    TrajectoryResult t;
    t.uav_id = id;
    t.status = TrajectoryStatus::Feasible;
    t.dependency_identity = id + "-trajectory";
    t.points = {
        {id + "-0", {0.0, 0.0}, 100.0, start, 0.0},
        {id + "-1", {0.0, 0.001}, 100.0, end, 10.0}
    };
    return t;
}
}

int main() {
    const auto a = makeTrajectory("A", 0.0, 10.0);
    const auto b = makeTrajectory("B", 0.0, 10.0);

    std::vector<MultiUavSeparationInput> separation = {
        {"A", 200.0, 20.0, 0.0},
        {"B", 200.0, 20.0, 0.0}
    };
    std::vector<MultiUavStartDelayInput> delays = {
        {"A", 0.0, 1.0, 0.0},
        {"B", 0.0, 1.0, 20.0}
    };

    const auto resolved = MultiUavSequencer::resolveByStartDelay(
        {a, b}, separation, delays, "1.0.0");
    assert(resolved.status == MultiUavSequencingStatus::Resolved);
    assert(resolved.scheduled_delays[1].initial_delay_s == 11.0);

    delays[1].maximum_delay_s = 5.0;
    const auto unresolved = MultiUavSequencer::resolveByStartDelay(
        {a, b}, separation, delays, "1.0.0");
    assert(unresolved.status == MultiUavSequencingStatus::Unresolved);

    return 0;
}
