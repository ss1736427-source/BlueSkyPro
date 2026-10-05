#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace bluesky::c2 {

enum class C2LinkState {
    Disconnected,
    Connecting,
    Connected,
    Degraded,
    Recovering,
    Failed
};

enum class C2ErrorCode {
    None,
    NotConnected,
    AuthenticationFailed,
    InvalidFrame,
    Unsupported,
    Timeout,
    LinkLost,
    TransportError,
    ProtocolError
};

struct C2Identity {
    std::string endpointId;
    std::string transport;
    std::string protocol;
    std::string protocolVersion;
};

struct C2LinkMetrics {
    double latencyMs{0.0};
    double packetLoss{0.0};
    double jitterMs{0.0};
    double bitrateKbps{0.0};
    std::int64_t lastRxTimestampMs{0};
    std::int64_t lastTxTimestampMs{0};
    bool healthy{false};
};

struct C2Frame {
    std::uint64_t sequence{0};
    std::int64_t timestampMs{0};
    std::string channel;
    std::string payload;
};

struct C2Result {
    std::uint64_t sequence{0};
    bool accepted{false};
    C2ErrorCode error{C2ErrorCode::None};
    std::string reason;
};

class UniversalC2Interface {
public:
    virtual ~UniversalC2Interface() = default;

    virtual C2Identity getIdentity() const = 0;
    virtual bool connect() = 0;
    virtual void disconnect() = 0;
    virtual C2LinkState getLinkState() const = 0;
    virtual C2LinkMetrics getLinkMetrics() const = 0;

    virtual C2Result send(const std::string& channel, const std::string& payload) = 0;
    virtual std::optional<C2Frame> receive() = 0;
    virtual C2Result heartbeat(std::int64_t timestampMs) = 0;
    virtual C2Result authenticate(const std::string& credentialReference) = 0;
};

} // namespace bluesky::c2
