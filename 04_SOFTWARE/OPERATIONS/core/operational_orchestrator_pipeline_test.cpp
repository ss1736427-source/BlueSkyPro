#include "operational_orchestrator_pipeline.hpp"

#include <cassert>

using namespace bluesky::operations;

namespace {

RuntimeDecisionContext ready_runtime() {
    RuntimeDecisionContext context;
    context.runtime_state_version = "runtime-1";
    context.active_mission_package_version = "package-1";
    context.actual_vehicle_state_version = "vehicle-1";
    context.runtime_state_current = true;
    context.active_package_consistent = true;
    context.energy_gate_ok = true;
    context.c2_operational = true;
    context.mode = RuntimeCoordinationMode::Continue;
    return context;
}

void test_runtime_gate_blocks_selection() {
    OperationalSnapshot snapshot;
    snapshot.actions.push_back(Action{"A1", {}, AutomationLevel::Automatic, true, true, true, true, 10});

    OrchestratorPipelineContext context;
    context.runtime = ready_runtime();
    context.runtime.safety_action_required = true;
    context.policy_version = "policy-1";
    context.input_state_version = "state-1";

    const auto result = run_orchestrator_pipeline(snapshot, context);
    assert(result.status == PipelineStatus::RuntimeBlocked);
    assert(result.selection.status == SelectionStatus::None);
    assert(result.evidence.outcome == DecisionOutcome::None);
}

void test_executable_selection_is_composed() {
    OperationalSnapshot snapshot;
    snapshot.actions.push_back(Action{"A2", {}, AutomationLevel::Automatic, true, true, true, true, 20});
    snapshot.actions.push_back(Action{"A1", {}, AutomationLevel::Automatic, true, true, true, true, 10});

    OrchestratorPipelineContext context;
    context.runtime = ready_runtime();
    context.selection.authority.system_authorized = true;
    context.policy_version = "policy-1";
    context.input_state_version = "state-1";

    const auto result = run_orchestrator_pipeline(snapshot, context);
    assert(result.status == PipelineStatus::Selected);
    assert(result.selection.action != nullptr);
    assert(result.selection.action->id == "A2");
    assert(result.evidence.outcome == DecisionOutcome::Selected);
    assert(result.evidence.considered_action_ids.size() == 2);
}

void test_human_required_is_preserved() {
    OperationalSnapshot snapshot;
    snapshot.actions.push_back(Action{"A3", {}, AutomationLevel::HumanGuided, true, true, true, true, 5});

    OrchestratorPipelineContext context;
    context.runtime = ready_runtime();
    context.selection.authority.role_authorized = true;

    const auto result = run_orchestrator_pipeline(snapshot, context);
    assert(result.status == PipelineStatus::HumanRequired);
    assert(result.selection.action != nullptr);
    assert(result.evidence.outcome == DecisionOutcome::HumanRequired);
}

} // namespace

int main() {
    test_runtime_gate_blocks_selection();
    test_executable_selection_is_composed();
    test_human_required_is_preserved();
    return 0;
}
