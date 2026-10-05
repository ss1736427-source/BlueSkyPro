#include "next_action_selection.hpp"

#include <cassert>

using namespace bluesky::operations;

int main() {
    OperationalSnapshot snapshot{
        {"battery"},
        {
            {"auto_high", {"battery"}, AutomationLevel::Automatic, true, true, true, true, 100},
            {"human_high", {"battery"}, AutomationLevel::HumanGuided, true, true, true, true, 100},
            {"blocked", {"missing"}, AutomationLevel::Automatic, true, true, true, true, 120},
            {"lower", {"battery"}, AutomationLevel::Automatic, true, true, true, true, 50}
        }
    };

    SelectionContext system{{true, true, true}};
    auto selected = select_next_action(snapshot, system);
    assert(selected.status == SelectionStatus::Selected);
    assert(selected.action != nullptr);
    assert(selected.action->id == "auto_high");
    assert(selected.reason == SelectionReason::Executable);
    assert(selected.resolution.authority == Authority::System);

    SelectionContext human{{false, true, false}};
    selected = select_next_action(snapshot, human);
    assert(selected.status == SelectionStatus::Selected);
    assert(selected.action != nullptr);
    assert(selected.action->id == "auto_high");
    assert(selected.reason == SelectionReason::HumanRequired);
    assert(selected.resolution.automation == AutomationLevel::HumanGuided);

    const auto parallel = select_parallel_actions(snapshot, system);
    assert(parallel.size() == 2);
    assert(parallel[0]->id == "auto_high");
    assert(parallel[1]->id == "human_high");

    OperationalSnapshot none{{"battery"}, {
        {"blocked", {"missing"}, AutomationLevel::Automatic, true, true, true, true, 100}
    }};
    selected = select_next_action(none, system);
    assert(selected.status == SelectionStatus::None);
    assert(selected.action == nullptr);

    return 0;
}
