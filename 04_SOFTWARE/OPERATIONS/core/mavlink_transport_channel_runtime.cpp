#include "mavlink_transport_channel_runtime.hpp"

#include <utility>

namespace bluesky::operations {

namespace {

std::optional<std::uint8_t> extractSequence(
    const std::vector<std::uint8_t>& frame) {
    if (frame.size() >= 6 && frame[0] == 0xFD) {
        return frame[4];
    }
    if (frame.size() >= 4 && frame[0] == 0xFE) {
        return frame[2];
    }
    return std::nullopt;
}

std::optional<std::pair<bool, std::int64_t>> extractTimesync(
    const std::vector<std::uint8_t>& frame) {
    std::size_t payload = 0;
    std::uint32_t message_id = 0;

    if (frame.size() >= 10 && frame[0] == 0xFD) {
        payload = 10;
        message_id = static_cast<std::uint32_t>(frame[7]) |
                     (static_cast<std::uint32_t>(frame[8]) << 8) |
                     (static_cast<std::uint32_t>(frame[9]) << 16);
    } else if (frame.size() >= 6 && frame[0] == 0xFE) {
        payload = 6;
        message_id = frame[5];
    } else {
        return std::nullopt;
    }

    if (message_id != 111 || frame.size() < payload + 16) {
        return std::nullopt;
    }

    auto read_i64 = [&](std::size_t offset) {
        std::uint64_t value = 0;
        for (std::size_t i = 0; i < 8; ++i) {
            value |= static_cast<std::uint64_t>(
                         frame[payload + offset + i]) << (8 * i);
        }
        return static_cast<std::int64_t>(value);
    };

    const auto tc1 = read_i64(0);
    const auto ts1 = read_i64(8);
    return std::make_pair(tc1 != 0, ts1);
}

} // namespace

MavlinkTransportChannelRuntime::MavlinkTransportChannelRuntime(
    std::int64_t heartbeat_timeout_ms)
    : heartbeat_timeout_ms_(heartbeat_timeout_ms),
      session_runtime_(heartbeat_timeout_ms) {}

bool MavlinkTransportChannelRuntime::registerChannel(
    const MavlinkTransportChannelConfig& config) {
    if (config.channel_id.empty() || config.session_id.empty() ||
        config.vehicle_id.empty() || config.source_id.empty() ||
        config.system_id == 0 || config.component_id == 0 ||
        channels_.find(config.channel_id) != channels_.end()) {
        return false;
    }

    if (config.transport == MavlinkTransportType::Udp &&
        !config.udp_local.has_value()) {
        return false;
    }

    const auto address_result = fleet_addressing_runtime_.registerBinding(
        FleetAddressBinding{
            config.vehicle_id,
            config.source_id,
            config.session_id,
            FleetAddress{config.system_id, config.component_id}});
    if (address_result != FleetAddressRegistrationResult::Registered) {
        return false;
    }

    ChannelState state;
    state.snapshot.config = config;
    state.snapshot.state = MavlinkTransportChannelState::Offline;
    if (config.transport == MavlinkTransportType::Udp) {
        state.udp_driver = std::make_unique<MavlinkUdpTransportDriver>();
    }
    channels_.emplace(config.channel_id, std::move(state));
    return true;
}

bool MavlinkTransportChannelRuntime::connect(const std::string& channel_id) {
    auto it = channels_.find(channel_id);
    if (it == channels_.end()) return false;

    auto& channel = it->second;
    if (channel.snapshot.state == MavlinkTransportChannelState::Connected) {
        return true;
    }

    channel.snapshot.state = MavlinkTransportChannelState::Connecting;

    if (channel.snapshot.config.transport == MavlinkTransportType::Udp) {
        if (!channel.udp_driver ||
            !channel.udp_driver->open(*channel.snapshot.config.udp_local)) {
            channel.snapshot.state = MavlinkTransportChannelState::Lost;
            ++channel.snapshot.stats.link_failures;
            return false;
        }
    }

    if (channel.snapshot.config.transport == MavlinkTransportType::Udp) {
        const auto local = channel.udp_driver->localEndpoint();
        if (!local.has_value()) {
            channel.udp_driver->close();
            channel.snapshot.state = MavlinkTransportChannelState::Lost;
            ++channel.snapshot.stats.link_failures;
            return false;
        }
        channel.snapshot.config.udp_local = *local;
    }

    channel.snapshot.state = MavlinkTransportChannelState::Connected;
    return true;
}

bool MavlinkTransportChannelRuntime::disconnect(
    const std::string& channel_id) {
    auto it = channels_.find(channel_id);
    if (it == channels_.end()) return false;

    auto& channel = it->second;
    if (channel.udp_driver) channel.udp_driver->close();
    channel.snapshot.state = MavlinkTransportChannelState::Offline;
    channel.rx_queue.clear();
    channel.tx_queue.clear();
    return true;
}

bool MavlinkTransportChannelRuntime::fail(const std::string& channel_id) {
    auto it = channels_.find(channel_id);
    if (it == channels_.end()) return false;

    auto& channel = it->second;
    if (channel.udp_driver) channel.udp_driver->close();
    channel.snapshot.state = MavlinkTransportChannelState::Lost;
    ++channel.snapshot.stats.link_failures;
    return true;
}

bool MavlinkTransportChannelRuntime::reconnect(
    const std::string& channel_id) {
    auto it = channels_.find(channel_id);
    if (it == channels_.end()) return false;

    auto& channel = it->second;
    channel.snapshot.state = MavlinkTransportChannelState::Recovering;
    channel.link_metrics.reset();
    channel.link_latency.reset();
    channel.link_bandwidth.reset();
    session_runtime_.reconnect(
        channel.snapshot.config.session_id,
        channel.snapshot.config.vehicle_id,
        channel.snapshot.config.source_id,
        channel.snapshot.config.system_id,
        channel.snapshot.config.component_id);

    if (channel.snapshot.config.transport == MavlinkTransportType::Udp) {
        if (!channel.udp_driver || !channel.udp_driver->open(*channel.snapshot.config.udp_local)) {
            channel.snapshot.state = MavlinkTransportChannelState::Lost;
            ++channel.snapshot.stats.link_failures;
            return false;
        }
        const auto local = channel.udp_driver->localEndpoint();
        if (!local.has_value()) {
            channel.udp_driver->close();
            channel.snapshot.state = MavlinkTransportChannelState::Lost;
            ++channel.snapshot.stats.link_failures;
            return false;
        }
        channel.snapshot.config.udp_local = *local;
    }

    ++channel.snapshot.stats.reconnects;
    channel.snapshot.state = MavlinkTransportChannelState::Connected;
    return true;
}

bool MavlinkTransportChannelRuntime::setUdpRemote(
    const std::string& channel_id, const MavlinkUdpEndpoint& remote) {
    auto it = channels_.find(channel_id);
    if (it == channels_.end() ||
        it->second.snapshot.config.transport != MavlinkTransportType::Udp ||
        remote.host.empty() || remote.port == 0) {
        return false;
    }
    it->second.snapshot.config.udp_remote = remote;
    return true;
}

bool MavlinkTransportChannelRuntime::setUdpIngressPeer(
    const std::string& channel_id, const MavlinkUdpEndpoint& peer) {
    auto it = channels_.find(channel_id);
    if (it == channels_.end() ||
        it->second.snapshot.config.transport != MavlinkTransportType::Udp ||
        peer.host.empty() || peer.port == 0) {
        return false;
    }
    it->second.snapshot.config.udp_ingress_peer = peer;
    return true;
}

bool MavlinkTransportChannelRuntime::send(
    const std::string& channel_id,
    std::int64_t timestamp_ms,
    const std::vector<std::uint8_t>& frame) {
    auto it = channels_.find(channel_id);
    if (it == channels_.end()) return false;

    auto& channel = it->second;
    if (channel.snapshot.state != MavlinkTransportChannelState::Connected &&
        channel.snapshot.state != MavlinkTransportChannelState::Degraded) {
        ++channel.snapshot.stats.transmit_failures;
        return false;
    }

    if (channel.snapshot.config.transport == MavlinkTransportType::Udp) {
        if (!channel.udp_driver ||
            !channel.snapshot.config.udp_remote.has_value() ||
            !channel.udp_driver->sendTo(
                *channel.snapshot.config.udp_remote, frame)) {
            ++channel.snapshot.stats.transmit_failures;
            return false;
        }
    } else {
        channel.tx_queue.push_back(frame);
    }

    ++channel.snapshot.stats.transmitted_frames;
    if (const auto timesync = extractTimesync(frame); timesync && !timesync->first) {
        channel.link_latency.observeProbeSent(timestamp_ms, timesync->second);
    }
    channel.snapshot.stats.last_transmit_timestamp_ms = timestamp_ms;
    channel.link_bandwidth.observeTransmit(frame.size(), timestamp_ms);
    return true;
}

bool MavlinkTransportChannelRuntime::injectReceive(
    const std::string& channel_id,
    std::int64_t timestamp_ms,
    const std::vector<std::uint8_t>& frame) {
    auto it = channels_.find(channel_id);
    if (it == channels_.end()) return false;

    auto& channel = it->second;
    if (channel.snapshot.state != MavlinkTransportChannelState::Connected &&
        channel.snapshot.state != MavlinkTransportChannelState::Degraded) {
        ++channel.snapshot.stats.rejected_frames;
        return false;
    }

    ++channel.snapshot.stats.received_frames;
    channel.snapshot.stats.last_receive_timestamp_ms = timestamp_ms;
    channel.rx_queue.push_back(frame);
    return true;
}

std::optional<std::vector<std::uint8_t>>
MavlinkTransportChannelRuntime::receive(const std::string& channel_id) {
    auto it = channels_.find(channel_id);
    if (it == channels_.end()) return std::nullopt;

    auto& channel = it->second;
    if (channel.rx_queue.empty()) return std::nullopt;

    const auto frame = channel.rx_queue.front();
    channel.rx_queue.erase(channel.rx_queue.begin());

    const auto result = session_runtime_.ingestRawFrame(
        channel.snapshot.config.session_id,
        channel.snapshot.config.dialect,
        channel.snapshot.config.vehicle_id,
        channel.snapshot.config.source_id,
        channel.snapshot.stats.last_receive_timestamp_ms,
        frame);

    if (result.accepted) {
        ++channel.snapshot.stats.accepted_frames;
        if (const auto sequence = extractSequence(frame)) {
            channel.link_metrics.observe(
                *sequence, channel.snapshot.stats.last_receive_timestamp_ms);
        }
        channel.link_bandwidth.observeReceive(frame.size(), channel.snapshot.stats.last_receive_timestamp_ms);
        if (const auto timesync = extractTimesync(frame); timesync && timesync->first) {
            channel.link_latency.observeTimesyncResponse(
                channel.snapshot.stats.last_receive_timestamp_ms, timesync->second);
        }
        if (result.snapshot.link_state == MavlinkLinkState::Degraded) {
            channel.snapshot.state = MavlinkTransportChannelState::Degraded;
        } else if (result.snapshot.link_state == MavlinkLinkState::Healthy) {
            channel.snapshot.state = MavlinkTransportChannelState::Connected;
        }
    } else {
        ++channel.snapshot.stats.rejected_frames;
        return std::nullopt;
    }

    return frame;
}

bool MavlinkTransportChannelRuntime::pollReceive(
    const std::string& channel_id, std::int64_t timestamp_ms) {
    auto it = channels_.find(channel_id);
    if (it == channels_.end()) return false;

    auto& channel = it->second;
    if (channel.snapshot.config.transport != MavlinkTransportType::Udp ||
        !channel.udp_driver ||
        (channel.snapshot.state != MavlinkTransportChannelState::Connected &&
         channel.snapshot.state != MavlinkTransportChannelState::Degraded)) {
        return false;
    }

    MavlinkUdpEndpoint source;
    const auto frame = channel.udp_driver->receive(&source);
    if (!frame.has_value()) return false;

    ++channel.snapshot.stats.received_frames;
    channel.snapshot.stats.last_receive_timestamp_ms = timestamp_ms;
    channel.snapshot.stats.last_receive_endpoint = source;

    if (channel.snapshot.config.udp_ingress_peer.has_value() &&
        !(source.host == channel.snapshot.config.udp_ingress_peer->host &&
          source.port == channel.snapshot.config.udp_ingress_peer->port)) {
        ++channel.snapshot.stats.rejected_frames;
        return false;
    }

    channel.rx_queue.push_back(*frame);
    return receive(channel_id).has_value();
}


std::optional<std::string> MavlinkTransportChannelRuntime::routeIncomingFrame(
    std::int64_t timestamp_ms,
    const std::vector<std::uint8_t>& frame) {
    if (timestamp_ms <= 0) return std::nullopt;

    std::optional<FleetAddress> address;
    if (frame.size() >= 7 && frame[0] == 0xFD) {
        address = FleetAddress{frame[5], frame[6]};
    } else if (frame.size() >= 5 && frame[0] == 0xFE) {
        address = FleetAddress{frame[3], frame[4]};
    } else {
        return std::nullopt;
    }

    const auto binding = fleet_addressing_runtime_.resolveAddress(*address);
    if (!binding.has_value()) return std::nullopt;

    for (auto& [channel_id, channel] : channels_) {
        const auto& config = channel.snapshot.config;
        if (config.vehicle_id != binding->vehicle_id ||
            config.source_id != binding->source_id ||
            config.session_id != binding->session_id ||
            config.system_id != address->system_id ||
            config.component_id != address->component_id) {
            continue;
        }

        if (channel.snapshot.state != MavlinkTransportChannelState::Connected &&
            channel.snapshot.state != MavlinkTransportChannelState::Degraded) {
            ++channel.snapshot.stats.rejected_frames;
            return std::nullopt;
        }

        if (!injectReceive(channel_id, timestamp_ms, frame)) {
            return std::nullopt;
        }
        return channel_id;
    }

    return std::nullopt;
}

MavlinkSessionSnapshot MavlinkTransportChannelRuntime::tickSession(
    const std::string& session_id, std::int64_t now_ms) {
    const auto snapshot = session_runtime_.tick(session_id, now_ms);

    for (auto& [_, channel] : channels_) {
        if (channel.snapshot.config.session_id != session_id) continue;
        if (snapshot.link_state == MavlinkLinkState::Lost) {
            channel.snapshot.state = MavlinkTransportChannelState::Lost;
        } else if (snapshot.link_state == MavlinkLinkState::Degraded) {
            channel.snapshot.state = MavlinkTransportChannelState::Degraded;
        } else if (snapshot.link_state == MavlinkLinkState::Healthy) {
            channel.snapshot.state = MavlinkTransportChannelState::Connected;
        }
    }

    return snapshot;
}

std::optional<MavlinkTransportChannelSnapshot>
MavlinkTransportChannelRuntime::snapshot(
    const std::string& channel_id) const {
    const auto it = channels_.find(channel_id);
    if (it == channels_.end()) return std::nullopt;

    auto result = it->second.snapshot;
    result.link_metrics = it->second.link_metrics.snapshot();
    result.link_latency = it->second.link_latency.snapshot();
    result.link_bandwidth = it->second.link_bandwidth.snapshot();
    result.session = session_runtime_.snapshot(
        result.config.session_id);
    return result;
}

} // namespace bluesky::operations
