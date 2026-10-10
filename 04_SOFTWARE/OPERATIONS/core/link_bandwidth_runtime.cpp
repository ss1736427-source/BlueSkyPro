#include "link_bandwidth_runtime.hpp"

namespace bluesky::operations {

namespace {

std::optional<double> rate(std::uint64_t bytes,
                          std::int64_t first_timestamp_ms,
                          std::int64_t last_timestamp_ms) {
    if (bytes == 0 || first_timestamp_ms <= 0 || last_timestamp_ms <= first_timestamp_ms) {
        return std::nullopt;
    }
    const auto elapsed_ms = last_timestamp_ms - first_timestamp_ms;
    return static_cast<double>(bytes) * 1000.0 /
           static_cast<double>(elapsed_ms);
}

} // namespace

void LinkBandwidthRuntime::reset() {
    snapshot_ = {};
}

bool LinkBandwidthRuntime::observeTransmit(std::size_t bytes,
                                           std::int64_t timestamp_ms) {
    if (bytes == 0 || timestamp_ms <= 0) return false;
    if (snapshot_.last_transmit_timestamp_ms != 0 &&
        timestamp_ms < snapshot_.last_transmit_timestamp_ms) return false;

    if (snapshot_.first_transmit_timestamp_ms == 0) {
        snapshot_.first_transmit_timestamp_ms = timestamp_ms;
    }
    snapshot_.transmitted_bytes += static_cast<std::uint64_t>(bytes);
    snapshot_.last_transmit_timestamp_ms = timestamp_ms;
    return true;
}

bool LinkBandwidthRuntime::observeReceive(std::size_t bytes,
                                          std::int64_t timestamp_ms) {
    if (bytes == 0 || timestamp_ms <= 0) return false;
    if (snapshot_.last_receive_timestamp_ms != 0 &&
        timestamp_ms < snapshot_.last_receive_timestamp_ms) return false;

    if (snapshot_.first_receive_timestamp_ms == 0) {
        snapshot_.first_receive_timestamp_ms = timestamp_ms;
    }
    snapshot_.received_bytes += static_cast<std::uint64_t>(bytes);
    snapshot_.last_receive_timestamp_ms = timestamp_ms;
    return true;
}

LinkBandwidthSnapshot LinkBandwidthRuntime::snapshot() const {
    return snapshot_;
}

std::optional<double> LinkBandwidthSnapshot::transmitRateBytesPerSecond() const {
    return rate(transmitted_bytes, first_transmit_timestamp_ms,
                last_transmit_timestamp_ms);
}

std::optional<double> LinkBandwidthSnapshot::receiveRateBytesPerSecond() const {
    return rate(received_bytes, first_receive_timestamp_ms,
                last_receive_timestamp_ms);
}

} // namespace bluesky::operations
