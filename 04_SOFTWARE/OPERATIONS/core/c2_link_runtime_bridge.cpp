#include "c2_link_runtime_bridge.hpp"

namespace bluesky::operations {

C2LinkMeasurement C2LinkRuntimeBridge::fromRuntimeSnapshot(
    const MavlinkTransportChannelSnapshot& transport) {
    C2LinkMeasurement measurement;

    if (transport.link_latency.last_rtt_ms.has_value()) {
        measurement.latency_ms =
            static_cast<double>(*transport.link_latency.last_rtt_ms);
    }
    measurement.packet_loss =
        transport.link_metrics.packetLossRatio();

    const auto tx = transport.link_bandwidth.transmitRateBytesPerSecond();
    const auto rx = transport.link_bandwidth.receiveRateBytesPerSecond();
    if (tx.has_value() || rx.has_value()) {
        const double bytes_per_second =
            tx.value_or(0.0) + rx.value_or(0.0);
        measurement.observed_bandwidth_kbps =
            bytes_per_second * 8.0 / 1000.0;
    }

    measurement.measured_timestamp_ms =
        transport.stats.last_receive_timestamp_ms != 0
            ? transport.stats.last_receive_timestamp_ms
            : transport.stats.last_transmit_timestamp_ms;
    return measurement;
}

bluesky::c2::ChannelState C2LinkRuntimeBridge::mapState(
    MavlinkTransportChannelState state) {
    switch (state) {
    case MavlinkTransportChannelState::Connected:
        return bluesky::c2::ChannelState::Active;
    case MavlinkTransportChannelState::Degraded:
        return bluesky::c2::ChannelState::Degraded;
    case MavlinkTransportChannelState::Recovering:
        return bluesky::c2::ChannelState::Recovering;
    case MavlinkTransportChannelState::Lost:
        return bluesky::c2::ChannelState::Failed;
    case MavlinkTransportChannelState::Offline:
    case MavlinkTransportChannelState::Connecting:
        return bluesky::c2::ChannelState::Unavailable;
    }
    return bluesky::c2::ChannelState::Unavailable;
}

std::optional<bluesky::c2::ChannelSnapshot>
C2LinkRuntimeBridge::toChannelSnapshot(
    const MavlinkTransportChannelSnapshot& transport,
    const C2LinkMeasurement& measurement) {
    if (transport.config.channel_id.empty() ||
        measurement.measured_timestamp_ms <= 0 ||
        measurement.latency_ms < 0.0 ||
        measurement.packet_loss < 0.0 ||
        measurement.packet_loss > 1.0 ||
        measurement.capacity_kbps < 0.0) {
        return std::nullopt;
    }

    bluesky::c2::ChannelSnapshot result;
    result.id = transport.config.channel_id;
    result.transport = transport.config.transport == MavlinkTransportType::Udp
        ? "UDP"
        : "MAVLINK";
    result.state = mapState(transport.state);
    result.latencyMs = measurement.latency_ms;
    result.packetLoss = measurement.packet_loss;
    result.capacityKbps = measurement.capacity_kbps;
    result.integrityOk = measurement.integrity_ok;
    result.authenticated = measurement.authenticated;
    result.priority = transport.config.priority;
    result.lastSuccessTimestampMs = measurement.measured_timestamp_ms;
    return result;
}

} // namespace bluesky::operations
