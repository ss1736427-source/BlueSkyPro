#pragma once

#include <cstdint>
#include <optional>

namespace bluesky::operations {

struct LinkBandwidthSnapshot {
    std::uint64_t transmitted_bytes{0};
    std::uint64_t received_bytes{0};
    std::int64_t first_transmit_timestamp_ms{0};
    std::int64_t last_transmit_timestamp_ms{0};
    std::int64_t first_receive_timestamp_ms{0};
    std::int64_t last_receive_timestamp_ms{0};

    std::optional<double> transmitRateBytesPerSecond() const;
    std::optional<double> receiveRateBytesPerSecond() const;
};

class LinkBandwidthRuntime final {
public:
    void reset();
    bool observeTransmit(std::size_t bytes, std::int64_t timestamp_ms);
    bool observeReceive(std::size_t bytes, std::int64_t timestamp_ms);
    LinkBandwidthSnapshot snapshot() const;

private:
    LinkBandwidthSnapshot snapshot_;
};

} // namespace bluesky::operations
