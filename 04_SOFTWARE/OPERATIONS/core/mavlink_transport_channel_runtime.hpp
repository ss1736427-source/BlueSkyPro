#pragma once

#include "fleet_addressing_runtime.hpp"
#include "link_metrics_runtime.hpp"
#include "link_latency_runtime.hpp"
#include "link_bandwidth_runtime.hpp"
#include "mavlink_session_runtime.hpp"
#include "mavlink_udp_transport_driver.hpp"
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace bluesky::operations {

enum class MavlinkTransportChannelState {
    Offline, Connecting, Connected, Degraded, Lost, Recovering
};

enum class MavlinkTransportType {
    DeterministicMemory, Serial, Udp, Tcp, Radio, Custom
};

struct MavlinkTransportChannelConfig {
    std::string channel_id;
    std::string session_id;
    std::string vehicle_id;
    std::string source_id;
    MavlinkDialect dialect{MavlinkDialect::PX4};
    std::uint8_t system_id{0};
    std::uint8_t component_id{0};
    MavlinkTransportType transport{MavlinkTransportType::DeterministicMemory};
    int priority{0};
    std::optional<MavlinkUdpEndpoint> udp_local;
    std::optional<MavlinkUdpEndpoint> udp_remote;
    std::optional<MavlinkUdpEndpoint> udp_ingress_peer;
};

struct MavlinkTransportChannelStats {
    std::uint64_t received_frames{0}, accepted_frames{0}, rejected_frames{0};
    std::uint64_t transmitted_frames{0}, transmit_failures{0};
    std::uint64_t reconnects{0}, link_failures{0};
    std::int64_t last_receive_timestamp_ms{0};
    std::int64_t last_transmit_timestamp_ms{0};
    std::optional<MavlinkUdpEndpoint> last_receive_endpoint;
};

struct MavlinkTransportChannelSnapshot {
    MavlinkTransportChannelConfig config;
    MavlinkTransportChannelState state{MavlinkTransportChannelState::Offline};
    MavlinkTransportChannelStats stats;
    LinkMetricsSnapshot link_metrics;
    LinkLatencySnapshot link_latency;
    LinkBandwidthSnapshot link_bandwidth;
    std::optional<MavlinkSessionSnapshot> session;
};

class MavlinkTransportChannelRuntime final {
public:
    explicit MavlinkTransportChannelRuntime(std::int64_t heartbeat_timeout_ms = 3000);

    bool registerChannel(const MavlinkTransportChannelConfig& config);
    bool connect(const std::string& channel_id);
    bool disconnect(const std::string& channel_id);
    bool fail(const std::string& channel_id);
    bool reconnect(const std::string& channel_id);
    bool setUdpRemote(const std::string& channel_id, const MavlinkUdpEndpoint& remote);
    bool setUdpIngressPeer(const std::string& channel_id,
                           const MavlinkUdpEndpoint& peer);

    bool send(const std::string& channel_id, std::int64_t timestamp_ms,
              const std::vector<std::uint8_t>& frame);
    bool injectReceive(const std::string& channel_id, std::int64_t timestamp_ms,
                       const std::vector<std::uint8_t>& frame);
    std::optional<std::vector<std::uint8_t>> receive(const std::string& channel_id);
    bool pollReceive(const std::string& channel_id, std::int64_t timestamp_ms);

    // Route an already received MAVLink frame by its system/component address.
    // Address admission remains owned by FleetAddressingRuntime; session validation
    // remains owned by MavlinkSessionRuntime.
    std::optional<std::string> routeIncomingFrame(
        std::int64_t timestamp_ms,
        const std::vector<std::uint8_t>& frame);

    MavlinkSessionSnapshot tickSession(const std::string& session_id,
                                       std::int64_t now_ms);
    std::optional<MavlinkTransportChannelSnapshot> snapshot(
        const std::string& channel_id) const;

private:
    struct ChannelState {
        MavlinkTransportChannelSnapshot snapshot;
        LinkMetricsRuntime link_metrics;
        LinkLatencyRuntime link_latency;
        LinkBandwidthRuntime link_bandwidth;
        std::vector<std::vector<std::uint8_t>> rx_queue;
        std::vector<std::vector<std::uint8_t>> tx_queue;
        std::unique_ptr<MavlinkUdpTransportDriver> udp_driver;
    };

    std::int64_t heartbeat_timeout_ms_;
    std::unordered_map<std::string, ChannelState> channels_;
    MavlinkSessionRuntime session_runtime_;
    FleetAddressingRuntime fleet_addressing_runtime_;
};

} // namespace bluesky::operations
