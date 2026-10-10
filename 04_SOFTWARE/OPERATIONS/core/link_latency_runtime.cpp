#include "link_latency_runtime.hpp"

namespace bluesky::operations {

void LinkLatencyRuntime::reset() {
    snapshot_ = {};
    pending_probes_.clear();
}

bool LinkLatencyRuntime::observeProbeSent(
    std::int64_t timestamp_ms, std::int64_t probe_timestamp_ns) {
    if (timestamp_ms <= 0 || probe_timestamp_ns <= 0) return false;
    pending_probes_[probe_timestamp_ns] = timestamp_ms;
    ++snapshot_.probes_sent;
    return true;
}

bool LinkLatencyRuntime::observeTimesyncResponse(
    std::int64_t receive_timestamp_ms, std::int64_t echoed_probe_timestamp_ns) {
    if (receive_timestamp_ms <= 0 || echoed_probe_timestamp_ns <= 0) return false;

    const auto it = pending_probes_.find(echoed_probe_timestamp_ns);
    if (it == pending_probes_.end()) {
        ++snapshot_.unmatched_responses;
        return false;
    }

    const auto rtt_ms = receive_timestamp_ms - it->second;
    pending_probes_.erase(it);
    if (rtt_ms < 0) return false;

    ++snapshot_.responses_received;
    snapshot_.last_rtt_ms = rtt_ms;
    snapshot_.last_measurement_timestamp_ms = receive_timestamp_ms;

    if (!snapshot_.min_rtt_ms || rtt_ms < *snapshot_.min_rtt_ms)
        snapshot_.min_rtt_ms = rtt_ms;
    if (!snapshot_.max_rtt_ms || rtt_ms > *snapshot_.max_rtt_ms)
        snapshot_.max_rtt_ms = rtt_ms;
    return true;
}

LinkLatencySnapshot LinkLatencyRuntime::snapshot() const {
    return snapshot_;
}

} // namespace bluesky::operations
