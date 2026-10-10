#include "mavlink_session_runtime.hpp"

namespace bluesky::operations {
MavlinkSessionRuntime::MavlinkSessionRuntime(std::int64_t timeout): heartbeat_timeout_ms_(timeout) {}
std::optional<std::uint8_t> MavlinkSessionRuntime::frameSequence(const std::vector<std::uint8_t>& f) {
    if (f.size() < 10 || f[0] != 0xFD) return std::nullopt;
    return f[4];
}
MavlinkSessionIngestResult MavlinkSessionRuntime::ingestRawFrame(
    const std::string& sid, MavlinkDialect dialect, const std::string& vehicle,
    const std::string& source, std::int64_t received, const std::vector<std::uint8_t>& frame) {
    MavlinkSessionIngestResult out;
    const auto seq = frameSequence(frame);
    if (!seq) return out;
    const auto decoded = MavlinkRawDecoderBoundary::decodeFrame(dialect, vehicle, source, received, frame);
    if (!decoded) return out;
    auto it = sessions_.find(sid);
    bool fresh = false;
    if (it == sessions_.end()) {
        SessionState state;
        state.snapshot.key = {sid, vehicle, source, decoded->system_id, decoded->component_id};
        it = sessions_.emplace(sid, std::move(state)).first;
        fresh = true;
    }
    auto& state = it->second;
    if (state.snapshot.key.vehicle_id != vehicle ||
        state.snapshot.key.source_id != source ||
        state.snapshot.key.system_id != decoded->system_id ||
        state.snapshot.key.component_id != decoded->component_id) return out;
    out = ingestDecoded(state, *seq, received, *decoded);
    out.new_session = fresh;
    return out;
}
MavlinkSessionIngestResult MavlinkSessionRuntime::ingestDecoded(
    SessionState& state, std::uint8_t seq, std::int64_t received, DecodedMavlinkMessage msg) {
    MavlinkSessionIngestResult out;
    if (state.snapshot.sequence_initialized) {
        const auto expected = static_cast<std::uint8_t>(state.snapshot.last_sequence + 1U);
        if (seq == state.snapshot.last_sequence) {
            ++state.snapshot.duplicates;
            out.sequence_result = MavlinkSequenceResult::Duplicate;
            out.snapshot = state.snapshot;
            return out;
        }
        if (seq != expected) {
            state.snapshot.packets_lost += static_cast<std::uint8_t>(seq - expected);
            out.sequence_result = MavlinkSequenceResult::Gap;
            state.snapshot.link_state = MavlinkLinkState::Degraded;
        } else out.sequence_result = MavlinkSequenceResult::InOrder;
    } else {
        state.snapshot.sequence_initialized = true;
        out.sequence_result = MavlinkSequenceResult::First;
    }
    state.snapshot.last_sequence = seq;
    ++state.snapshot.packets_received;
    state.snapshot.last_received_timestamp_ms = received;
    out.message = msg;
    out.accepted = true;
    if (msg.kind == MavlinkMessageKind::Heartbeat) {
        state.snapshot.heartbeat_seen = true;
        state.snapshot.heartbeat_healthy = msg.healthy;
        state.snapshot.last_heartbeat_timestamp_ms = received;
        state.snapshot.link_state = msg.healthy ? MavlinkLinkState::Healthy : MavlinkLinkState::Degraded;
    }
    out.snapshot = state.snapshot;
    return out;
}
MavlinkSessionSnapshot MavlinkSessionRuntime::tick(const std::string& sid, std::int64_t now) {
    const auto it = sessions_.find(sid);
    if (it == sessions_.end()) return {};
    auto& s = it->second.snapshot;
    if (s.last_heartbeat_timestamp_ms > 0 && now - s.last_heartbeat_timestamp_ms > heartbeat_timeout_ms_)
        s.link_state = MavlinkLinkState::Lost;
    return s;
}
MavlinkSessionSnapshot MavlinkSessionRuntime::reconnect(
    const std::string& sid, const std::string& vehicle, const std::string& source,
    std::uint8_t sysid, std::uint8_t compid) {
    SessionState s;
    s.snapshot.key = {sid, vehicle, source, sysid, compid};
    s.snapshot.link_state = MavlinkLinkState::Recovering;
    sessions_[sid] = std::move(s);
    return sessions_[sid].snapshot;
}
std::optional<MavlinkSessionSnapshot> MavlinkSessionRuntime::snapshot(const std::string& sid) const {
    const auto it = sessions_.find(sid);
    if (it == sessions_.end()) return std::nullopt;
    return it->second.snapshot;
}
}
