#pragma once

#include "ph4_int_009_flight_record_producer.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace bluesky::flight_record {

enum class LifecycleState {
    Init,
    Preparing,
    Active,
    Completed,
    Aborted
};

struct LifecycleEvent {
    std::string flightRecordId;
    std::int64_t timestampMs{};
    std::string eventType;
    LifecycleState state{LifecycleState::Init};
};

class FlightRecordLifecycle final {
public:
    explicit FlightRecordLifecycle(FlightRecordReference reference);

    LifecycleEvent beginPreparation();
    LifecycleEvent beginExecution();
    LifecycleEvent completeExecution();
    LifecycleEvent abortExecution();

    LifecycleState state() const { return state_; }
    const std::vector<LifecycleEvent>& events() const { return events_; }

private:
    LifecycleEvent transition(LifecycleState next, std::string eventType);
    static bool allowed(LifecycleState from, LifecycleState to);
    static std::string stateName(LifecycleState state);

    FlightRecordReference reference_;
    LifecycleState state_{LifecycleState::Init};
    std::vector<LifecycleEvent> events_;
};

} // namespace bluesky::flight_record
