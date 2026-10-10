#include "runtime_decision_context.hpp"

#include <cassert>

using namespace bluesky::operations;

int main() {
    RuntimeDecisionContext valid;
    valid.runtime_state_version = "runtime-42";
    valid.active_mission_package_version = "package-7";
    valid.actual_vehicle_state_version = "vehicle-91";
    valid.runtime_state_current = true;
    valid.active_package_consistent = true;
    valid.energy_gate_ok = true;
    valid.c2_operational = true;

    assert(decision_context_usable(valid));
    assert(operational_selection_allowed(valid));

    auto stale = valid;
    stale.runtime_state_current = false;
    assert(!decision_context_usable(stale));
    assert(!operational_selection_allowed(stale));

    auto inconsistent = valid;
    inconsistent.active_package_consistent = false;
    assert(!decision_context_usable(inconsistent));

    auto energy_blocked = valid;
    energy_blocked.energy_gate_ok = false;
    assert(decision_context_usable(energy_blocked));
    assert(!operational_selection_allowed(energy_blocked));

    auto recovery = valid;
    recovery.mode = RuntimeCoordinationMode::Recover;
    recovery.safety_action_required = true;
    assert(decision_context_usable(recovery));
    assert(!operational_selection_allowed(recovery));

    auto safety_action = valid;
    safety_action.safety_action_required = true;
    assert(!operational_selection_allowed(safety_action));

    return 0;
}
