#include "safety_failsafe_adapter_baseline.hpp"

#include <utility>

namespace bluesky::safety {

SafetyFailsafeAdapterBaseline::SafetyFailsafeAdapterBaseline(std::string sourceId)
    : sourceId_(std::move(sourceId)) {}

std::string SafetyFailsafeAdapterBaseline::getSourceId() const {
    return sourceId_;
}

std::string SafetyFailsafeAdapterBaseline::getConfigurationVersion() const {
    return "BASELINE-1";
}

SafetyState SafetyFailsafeAdapterBaseline::getState() const {
    return event_.has_value() ? event_->state : SafetyState::Unknown;
}

SafetyQuality SafetyFailsafeAdapterBaseline::getQuality() const {
    return event_.has_value() ? event_->quality : SafetyQuality::Unknown;
}

SafetyEvent SafetyFailsafeAdapterBaseline::readEvent() const {
    return event_.value_or(SafetyEvent{});
}

SafetyResult SafetyFailsafeAdapterBaseline::acceptEvent(const SafetyEvent& event) {
    if (sourceId_.empty() || event.sourceId != sourceId_) {
        return {false, SafetyError::ConfigurationMismatch, "source_id mismatch"};
    }
    if (event.timestampUtcMs <= 0) {
        return {false, SafetyError::InvalidData, "timestamp must be positive"};
    }
    if (!event.valid) {
        return {false, SafetyError::InvalidData, "event is not valid"};
    }
    if (event.stale) {
        return {false, SafetyError::StaleData, "event is stale"};
    }
    if (event.trigger == SafetyTrigger::None) {
        return {false, SafetyError::InvalidData, "trigger is required"};
    }
    if (event.requestedAction == SafetyAction::None) {
        return {false, SafetyError::InvalidData, "requested action is required"};
    }
    if (event.appliedAction == SafetyAction::None) {
        return {false, SafetyError::InvalidData, "applied action is required"};
    }
    if (event.state == SafetyState::Unknown || event.quality == SafetyQuality::Unknown) {
        return {false, SafetyError::InvalidData, "state and quality are required"};
    }
    event_ = event;
    return {true, SafetyError::None, "accepted"};
}

} // namespace bluesky::safety
