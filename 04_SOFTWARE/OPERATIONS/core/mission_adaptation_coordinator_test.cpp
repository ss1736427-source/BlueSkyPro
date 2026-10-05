#include "mission_adaptation_coordinator.hpp"

#include <cassert>

using namespace bluesky::operations;

int main() {
    AdaptationCandidate alternate{
        "alternate-uav", AdaptationType::AlternateVehicle,
        {"uav-02"}, {"task-01"}, true, true, true, true, true, 100};
    AdaptationCandidate redistribution{
        "redistribute", AdaptationType::TaskRedistribution,
        {"uav-02", "uav-03"}, {"task-01", "task-02"}, true, true, true, true, true, 80};

    const std::vector<AdaptationCandidate> candidates{redistribution, alternate};
    const AdaptationContext authorized{true};
    const auto selected = select_adaptation(candidates, authorized);

    assert(selected.status == AdaptationStatus::Proposed);
    assert(selected.candidate != nullptr);
    assert(selected.candidate->id == "alternate-uav");

    AdaptationCandidate outside_envelope{
        "route-change", AdaptationType::RouteProfileChange,
        {"uav-02"}, {"task-01"}, false, true, true, true, true, 100};
    const AdaptationContext no_authority{false};
    const auto human = select_adaptation({outside_envelope}, no_authority);
    assert(human.status == AdaptationStatus::RequiresHuman);
    assert(human.candidate != nullptr);
    assert(human.candidate->id == "route-change");

    AdaptationCandidate unsafe{
        "unsafe", AdaptationType::SpeedAltitudeChange,
        {"uav-02"}, {"task-01"}, true, false, true, true, true, 200};
    const auto blocked = select_adaptation({unsafe}, authorized);
    assert(blocked.status == AdaptationStatus::Infeasible);
    assert(blocked.candidate == nullptr);

    return 0;
}
