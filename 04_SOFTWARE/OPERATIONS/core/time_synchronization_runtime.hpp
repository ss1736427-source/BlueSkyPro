#pragma once

#include <cstdint>
#include <optional>

namespace bluesky::operations {

enum class TimeSynchronizationState {
    LocalOnly,
    Synchronizing,
    Synchronized,
    Stale
};

struct TimeSynchronizationSnapshot {
    TimeSynchronizationState state{TimeSynchronizationState::LocalOnly};
    std::uint64_t observations{0};
    std::optional<std::int64_t> offset_ms;
    std::optional<std::int64_t> last_rtt_ms;
    std::int64_t last_observation_timestamp_ms{0};
};

class TimeSynchronizationRuntime final {
public:
    void reset();

    bool observe(std::int64_t local_send_timestamp_ms,
                 std::int64_t local_receive_timestamp_ms,
                 std::int64_t remote_timestamp_ms);

    void markStale(std::int64_t now_ms, std::int64_t stale_after_ms);

    TimeSynchronizationSnapshot snapshot() const;

private:
    TimeSynchronizationSnapshot snapshot_;
};

} // namespace bluesky::operations
