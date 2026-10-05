#include "safety_reconciliation.hpp"
#include "safety_failsafe_adapter_baseline.hpp"
#include <cassert>
#include <vector>
using namespace bluesky::safety;

int main() {
    SafetyFailsafeAdapterBaseline adapter;
    SafetyEvent event;
    event.sourceId="SRC-19"; event.timestampUtcMs=1000;
    event.trigger=SafetyTrigger::C2Loss;
    event.requestedAction=SafetyAction::ReturnToOperator;
    event.appliedAction=SafetyAction::ReturnToOperator;
    event.state=SafetyState::Failsafe; event.quality=SafetyQuality::Valid; event.valid=true;
    assert(adapter.acceptEvent(event).accepted);

    auto matched=SafetyReconciliation::evaluate(
        {{SafetyConstraintKind::C2Loss,true,SafetyAction::ReturnToOperator}},adapter);
    assert(matched.releaseAllowed);
    assert(matched.items.front().status==ReconciliationStatus::Matched);

    auto mismatch=SafetyReconciliation::evaluate(
        {{SafetyConstraintKind::C2Loss,true,SafetyAction::Hold}},adapter);
    assert(!mismatch.releaseAllowed);
    assert(mismatch.items.front().status==ReconciliationStatus::Mismatch);

    auto unknown=SafetyReconciliation::evaluate(
        {{SafetyConstraintKind::C2Loss,true,SafetyAction::ReturnToOperator}},
        SafetyFailsafeAdapterBaseline{});
    assert(!unknown.releaseAllowed);
    assert(unknown.items.front().status==ReconciliationStatus::Unknown);
    return 0;
}
