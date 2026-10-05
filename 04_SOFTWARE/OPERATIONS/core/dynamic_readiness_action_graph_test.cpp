#include "dynamic_readiness_action_graph.hpp"

#include <cassert>

using namespace bluesky::operations;

int main() {
    OperationalSnapshot operational{{"battery_connected"}, {}};
    ReadinessSnapshot readiness{{
        {{"battery_connected", {}, AutomationLevel::Automatic, true, true, true, true, 100},
         "technician", true, true, false},
        {{"meteo_check", {}, AutomationLevel::Automatic, true, true, true, true, 90},
         "dispatcher", false, true, false},
        {{"release_authorization", {"meteo_check"}, AutomationLevel::HumanDecision,
          true, true, true, true, 80},
         "authorized_operator", false, true, true},
        {{"blocked_resource", {}, AutomationLevel::Automatic, true, true, true, true, 100},
         "technician", false, false, false},
        {{"blocked_dependency", {"unknown"}, AutomationLevel::Automatic, true, true, true, true, 95},
         "technician", false, true, false}
    }};

    assert(readiness_status(readiness.actions[0], operational) == ActionStatus::Blocked);
    assert(readiness_status(readiness.actions[1], operational) == ActionStatus::Actionable);
    assert(readiness_status(readiness.actions[2], operational) == ActionStatus::Blocked);
    assert(readiness_status(readiness.actions[3], operational) == ActionStatus::Blocked);
    assert(readiness_status(readiness.actions[4], operational) == ActionStatus::Blocked);

    const auto candidates = actionable_readiness(readiness, operational);
    assert(candidates.size() == 1);
    assert(candidates.front()->action.id == "meteo_check");

    ReadinessSnapshot parallel{{
        {{"z_action", {}, AutomationLevel::Automatic, true, true, true, true, 50},
         "dispatcher", false, true, false},
        {{"a_action", {}, AutomationLevel::Automatic, true, true, true, true, 50},
         "technician", false, true, false},
        {{"lower_priority", {}, AutomationLevel::Automatic, true, true, true, true, 40},
         "technician", false, true, false}
    }};
    const auto parallel_set = parallel_ready_set(parallel, operational);
    assert(parallel_set.size() == 2);
    assert(parallel_set[0]->action.id == "a_action");
    assert(parallel_set[1]->action.id == "z_action");

    ReadinessSnapshot completed{{
        {{"done", {}, AutomationLevel::Automatic, true, true, true, true, 100},
         "technician", true, true, false}
    }};
    assert(actionable_readiness(completed, operational).empty());

    return 0;
}
