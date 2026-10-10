#pragma once

#include <cstdint>
#include <optional>
#include <unordered_map>

namespace bluesky::operations {

struct LinkLatencySnapshot {
    std::uint64_t probes_sent{0};
    std::uint64_t responses_received{0};
    std::uint64_t unmatched_responses{0};
    std::optional<std::int64_t> last_rtt_ms;
    std::optional<std::int64_t> min_rtt_ms;
    std::optional<std::int64_t> max_rtt_ms;
    std::int64_t last_measurement_timestamp_ms{0};
};

class LinkLatencyRuntime final {
public:
    void reset();
    bool observeProbeSent(std::int64_t timestamp_ms, std::int64_t probe_timestamp_ns);
    bool observeTimesyncResponse(std::int64_t receive_timestamp_ms,
                                 std::int64_t echoed_probe_timestamp_ns);
    LinkLatencySnapshot snapshot() const;

private:
    LinkLatencySnapshot snapshot_;
    std::unordered_map<std::int64_t, std::int64_t> pending_probes_;
};

} // namespace bluesky::operations
