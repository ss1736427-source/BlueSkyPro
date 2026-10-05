#include "ph4_int_011_flight_record_lifecycle.hpp"

#include <chrono>
#include <stdexcept>
#include <utility>

namespace bluesky::flight_record {

namespace {
std::int64_t nowMs() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::system_clock::now().time_since_epoch())
        .count();
}
} // namespace

FlightRecordLifecycle::FlightRecordLifecycle(FlightRecordReference reference)
    : reference_(std::move(reference)) {
    if (reference_.flightRecordId.empty()) {
        throw std::invalid_argument("flight_record_id is required");
    }
}

bool FlightRecordLifecycle::allowed(LifecycleState from, LifecycleState to) {
    return (from == LifecycleState::Init && to == LifecycleState::Preparing) ||
           (from == LifecycleState::Preparing && to == LifecycleState::Active) ||
           (from == LifecycleState::Active && to == LifecycleState::Completed) ||
           (from == LifecycleState::Active && to == LifecycleState::Aborted);
}

std::string FlightRecordLifecycle::stateName(LifecycleState state) {
    switch (state) {
    case LifecycleState::Init: return "INIT";
    case LifecycleState::Preparing: return "PREPARING";
    case LifecycleState::Active: return "ACTIVE";
    case LifecycleState::Completed: return "COMPLETED";
    case LifecycleState::Aborted: return "ABORTED";
    }
    return "UNKNOWN";
}

LifecycleEvent FlightRecordLifecycle::transition(LifecycleState next, std::string eventType) {
    if (!allowed(state_, next)) {
        throw std::logic_error("invalid Flight Record lifecycle transition from " + stateName(state_) +
                               " to " + stateName(next));
    }
    state_ = next;
    LifecycleEvent event{reference_.flightRecordId, nowMs(), std::move(eventType), state_};
    events_.push_back(event);
    return event;
}

LifecycleEvent FlightRecordLifecycle::beginPreparation() {
    return transition(LifecycleState::Preparing, "MISSION_PREPARATION_STARTED");
}

LifecycleEvent FlightRecordLifecycle::beginExecution() {
    return transition(LifecycleState::Active, "MISSION_EXECUTION_STARTED");
}

LifecycleEvent FlightRecordLifecycle::completeExecution() {
    return transition(LifecycleState::Completed, "MISSION_EXECUTION_COMPLETED");
}

LifecycleEvent FlightRecordLifecycle::abortExecution() {
    return transition(LifecycleState::Aborted, "MISSION_EXECUTION_ABORTED");
}

} // namespace bluesky::flight_record
