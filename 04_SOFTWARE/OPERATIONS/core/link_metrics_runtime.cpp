#include "link_metrics_runtime.hpp"

#include <algorithm>

namespace bluesky::operations {

double LinkMetricsSnapshot::packetLossRatio() const {
    const auto denominator = observed_packets + inferred_lost_packets;
    if (denominator == 0) return 0.0;
    return static_cast<double>(inferred_lost_packets) /
           static_cast<double>(denominator);
}

std::optional<std::int64_t> LinkMetricsSnapshot::ageMs(
    std::int64_t now_ms) const {
    if (last_observation_timestamp_ms <= 0 || now_ms < last_observation_timestamp_ms) {
        return std::nullopt;
    }
    return now_ms - last_observation_timestamp_ms;
}

void LinkMetricsRuntime::reset() {
    snapshot_ = {};
}

bool LinkMetricsRuntime::observe(
    std::uint8_t sequence, std::int64_t timestamp_ms) {
    if (timestamp_ms <= 0) return false;

    if (!snapshot_.sequence_initialized) {
        snapshot_.sequence_initialized = true;
        snapshot_.last_sequence = sequence;
        snapshot_.first_observation_timestamp_ms = timestamp_ms;
        snapshot_.last_observation_timestamp_ms = timestamp_ms;
        snapshot_.observed_packets = 1;
        return true;
    }

    const std::uint8_t delta =
        static_cast<std::uint8_t>(sequence - snapshot_.last_sequence);

    if (delta == 0) {
        ++snapshot_.duplicate_packets;
        snapshot_.last_observation_timestamp_ms = timestamp_ms;
        return true;
    }

    if (delta < 128) {
        if (delta > 1) {
            snapshot_.inferred_lost_packets += static_cast<std::uint64_t>(delta - 1);
        }
        snapshot_.last_sequence = sequence;
        ++snapshot_.observed_packets;
        snapshot_.last_observation_timestamp_ms = timestamp_ms;
        return true;
    }

    ++snapshot_.out_of_order_packets;
    ++snapshot_.observed_packets;
    snapshot_.last_observation_timestamp_ms = timestamp_ms;
    return true;
}

LinkMetricsSnapshot LinkMetricsRuntime::snapshot() const {
    return snapshot_;
}

} // namespace bluesky::operations
