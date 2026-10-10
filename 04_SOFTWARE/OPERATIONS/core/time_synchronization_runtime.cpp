#include "time_synchronization_runtime.hpp"

namespace bluesky::operations {

void TimeSynchronizationRuntime::reset() {
    snapshot_ = {};
}

bool TimeSynchronizationRuntime::observe(std::int64_t local_send_timestamp_ms,
                                          std::int64_t local_receive_timestamp_ms,
                                          std::int64_t remote_timestamp_ms) {
    if (local_send_timestamp_ms <= 0 ||
        local_receive_timestamp_ms < local_send_timestamp_ms ||
        remote_timestamp_ms <= 0) {
        return false;
    }

    const auto midpoint_ms =
        local_send_timestamp_ms +
        (local_receive_timestamp_ms - local_send_timestamp_ms) / 2;
    const auto offset_ms = remote_timestamp_ms - midpoint_ms;
    const auto rtt_ms = local_receive_timestamp_ms - local_send_timestamp_ms;

    ++snapshot_.observations;
    snapshot_.offset_ms = offset_ms;
    snapshot_.last_rtt_ms = rtt_ms;
    snapshot_.last_observation_timestamp_ms = local_receive_timestamp_ms;
    snapshot_.state = TimeSynchronizationState::Synchronized;
    return true;
}

void TimeSynchronizationRuntime::markStale(std::int64_t now_ms,
                                           std::int64_t stale_after_ms) {
    if (now_ms <= 0 || stale_after_ms < 0 ||
        snapshot_.last_observation_timestamp_ms <= 0) {
        return;
    }

    if (now_ms - snapshot_.last_observation_timestamp_ms > stale_after_ms) {
        snapshot_.state = TimeSynchronizationState::Stale;
    }
}

TimeSynchronizationSnapshot TimeSynchronizationRuntime::snapshot() const {
    return snapshot_;
}

} // namespace bluesky::operations
