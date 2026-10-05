#include "safety_failsafe_adapter_baseline.hpp"

#include <cassert>

using namespace bluesky::safety;

int main() {
    SafetyFailsafeAdapterBaseline adapter;
    assert(adapter.getSourceId() == "SRC-19");
    assert(adapter.getConfigurationVersion() == "BASELINE-1");
    assert(adapter.getState() == SafetyState::Unknown);

    SafetyEvent event;
    event.sourceId = "SRC-19";
    event.timestampUtcMs = 1000;
    event.trigger = SafetyTrigger::C2Loss;
    event.requestedAction = SafetyAction::ReturnToOperator;
    event.appliedAction = SafetyAction::ReturnToOperator;
    event.state = SafetyState::Failsafe;
    event.quality = SafetyQuality::Valid;
    event.valid = true;

    const auto accepted = adapter.acceptEvent(event);
    assert(accepted.accepted);
    assert(accepted.error == SafetyError::None);
    assert(adapter.getState() == SafetyState::Failsafe);
    assert(adapter.getQuality() == SafetyQuality::Valid);
    assert(adapter.readEvent().appliedAction == SafetyAction::ReturnToOperator);

    auto invalid = event;
    invalid.timestampUtcMs = 0;
    assert(!adapter.acceptEvent(invalid).accepted);
    assert(adapter.acceptEvent(invalid).error == SafetyError::InvalidData);

    auto noTrigger = event;
    noTrigger.trigger = SafetyTrigger::None;
    assert(!adapter.acceptEvent(noTrigger).accepted);
    assert(adapter.acceptEvent(noTrigger).error == SafetyError::InvalidData);

    auto stale = event;
    stale.stale = true;
    assert(!adapter.acceptEvent(stale).accepted);
    assert(adapter.acceptEvent(stale).error == SafetyError::StaleData);

    return 0;
}
