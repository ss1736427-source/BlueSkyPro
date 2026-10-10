#include "operational_orchestrator_contract.hpp"

#include <cassert>

using namespace bluesky::operations;

int main() {
    OperationalSnapshot snapshot{
        {"connect_battery"},
        {
            {"connect_battery", {}, AutomationLevel::Automatic, true, true, true, true, 100},
            {"engine_test", {"connect_battery"}, AutomationLevel::HumanGuided, true, true, true, true, 90},
            {"release", {"engine_test"}, AutomationLevel::HumanDecision, true, true, true, true, 80},
            {"blocked_energy", {}, AutomationLevel::Automatic, true, true, true, false, 100}
        }
    };

    assert(evaluate(snapshot.actions[0], snapshot) == ActionStatus::Actionable);
    assert(evaluate(snapshot.actions[1], snapshot) == ActionStatus::Actionable);
    assert(evaluate(snapshot.actions[2], snapshot) == ActionStatus::Blocked);
    assert(evaluate(snapshot.actions[3], snapshot) == ActionStatus::Blocked);

    const auto selected = select_next(snapshot);
    assert(selected != nullptr);
    assert(selected->id == "connect_battery");

    OperationalSnapshot parallel{
        {},
        {
            {"b_action", {}, AutomationLevel::Automatic, true, true, true, true, 50},
            {"a_action", {}, AutomationLevel::Automatic, true, true, true, true, 50}
        }
    };
    const auto candidates = actionable(parallel);
    assert(candidates.size() == 2);
    assert(candidates[0]->id == "a_action");
    assert(candidates[1]->id == "b_action");

    return 0;
}
