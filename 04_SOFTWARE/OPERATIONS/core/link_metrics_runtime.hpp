#pragma once

#include <cstdint>
#include <optional>

namespace bluesky::operations {

struct LinkMetricsSnapshot {
    std::uint64_t observed_packets{0};
    std::uint64_t inferred_lost_packets{0};
    std::uint64_t duplicate_packets{0};
    std::uint64_t out_of_order_packets{0};
    std::uint8_t last_sequence{0};
    bool sequence_initialized{false};
    std::int64_t first_observation_timestamp_ms{0};
    std::int64_t last_observation_timestamp_ms{0};

    double packetLossRatio() const;
    std::optional<std::int64_t> ageMs(std::int64_t now_ms) const;
};

class LinkMetricsRuntime final {
public:
    void reset();

    // Observes a validated MAVLink packet on this link only.
    // The sequence counter is interpreted modulo 256.
    bool observe(std::uint8_t sequence, std::int64_t timestamp_ms);

    LinkMetricsSnapshot snapshot() const;

private:
    LinkMetricsSnapshot snapshot_;
};

} // namespace bluesky::operations
