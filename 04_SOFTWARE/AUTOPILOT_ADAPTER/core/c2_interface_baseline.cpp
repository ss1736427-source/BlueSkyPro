#include "c2_interface_baseline.hpp"

#include <utility>

namespace bluesky::c2 {

C2InterfaceBaseline::C2InterfaceBaseline(C2Identity identity)
    : identity_(std::move(identity)) {}

C2Identity C2InterfaceBaseline::getIdentity() const { return identity_; }

bool C2InterfaceBaseline::connect() {
    state_ = C2LinkState::Connecting;
    state_ = C2LinkState::Connected;
    metrics_.healthy = authenticated_;
    return true;
}

void C2InterfaceBaseline::disconnect() {
    state_ = C2LinkState::Disconnected;
    metrics_.healthy = false;
}

C2LinkState C2InterfaceBaseline::getLinkState() const { return state_; }
C2LinkMetrics C2InterfaceBaseline::getLinkMetrics() const { return metrics_; }

C2Result C2InterfaceBaseline::authenticate(const std::string& credentialReference) {
    C2Result result;
    result.sequence = nextSequence_;
    if (credentialReference.empty()) {
        result.error = C2ErrorCode::AuthenticationFailed;
        result.reason = "CREDENTIAL_REFERENCE_REQUIRED";
        return result;
    }
    authenticated_ = true;
    metrics_.healthy = state_ == C2LinkState::Connected;
    result.accepted = true;
    result.reason = "AUTHENTICATED";
    return result;
}

C2Result C2InterfaceBaseline::send(const std::string& channel, const std::string& payload) {
    C2Result result;
    result.sequence = ++nextSequence_;
    if (state_ != C2LinkState::Connected) {
        result.error = C2ErrorCode::NotConnected;
        result.reason = "C2_LINK_NOT_CONNECTED";
        return result;
    }
    if (channel.empty() || payload.empty()) {
        result.error = C2ErrorCode::InvalidFrame;
        result.reason = "CHANNEL_AND_PAYLOAD_REQUIRED";
        return result;
    }
    rxQueue_.push_back(C2Frame{result.sequence, metrics_.lastTxTimestampMs, channel, payload});
    result.accepted = true;
    result.reason = "FRAME_ACCEPTED";
    return result;
}

std::optional<C2Frame> C2InterfaceBaseline::receive() {
    if (rxQueue_.empty()) {
        return std::nullopt;
    }
    C2Frame frame = rxQueue_.front();
    rxQueue_.pop_front();
    return frame;
}

C2Result C2InterfaceBaseline::heartbeat(std::int64_t timestampMs) {
    C2Result result;
    result.sequence = ++nextSequence_;
    if (state_ != C2LinkState::Connected) {
        result.error = C2ErrorCode::LinkLost;
        result.reason = "C2_LINK_NOT_CONNECTED";
        return result;
    }
    metrics_.lastTxTimestampMs = timestampMs;
    metrics_.lastRxTimestampMs = timestampMs;
    metrics_.healthy = authenticated_;
    result.accepted = true;
    result.reason = "HEARTBEAT_OK";
    return result;
}

} // namespace bluesky::c2
