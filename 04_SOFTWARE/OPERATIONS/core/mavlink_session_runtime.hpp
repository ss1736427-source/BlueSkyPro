#pragma once

#include "mavlink_raw_decoder_boundary.hpp"
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace bluesky::operations {
enum class MavlinkLinkState { Healthy, Degraded, Lost, Recovering };
enum class MavlinkSequenceResult { First, InOrder, Duplicate, Gap };
struct MavlinkSessionKey {
    std::string session_id, vehicle_id, source_id;
    std::uint8_t system_id{0}, component_id{0};
    bool operator==(const MavlinkSessionKey&) const = default;
};
struct MavlinkSessionSnapshot {
    MavlinkSessionKey key;
    MavlinkLinkState link_state{MavlinkLinkState::Recovering};
    bool heartbeat_seen{false}, heartbeat_healthy{false}, sequence_initialized{false};
    std::uint8_t last_sequence{0};
    std::uint64_t packets_received{0}, packets_lost{0}, duplicates{0};
    std::int64_t last_received_timestamp_ms{0}, last_heartbeat_timestamp_ms{0};
};
struct MavlinkSessionIngestResult {
    MavlinkSessionSnapshot snapshot;
    MavlinkSequenceResult sequence_result{MavlinkSequenceResult::First};
    std::optional<DecodedMavlinkMessage> message;
    bool accepted{false}, new_session{false};
};
class MavlinkSessionRuntime final {
public:
    explicit MavlinkSessionRuntime(std::int64_t heartbeat_timeout_ms = 3000);
    MavlinkSessionIngestResult ingestRawFrame(const std::string&, MavlinkDialect,
        const std::string&, const std::string&, std::int64_t,
        const std::vector<std::uint8_t>&);
    MavlinkSessionSnapshot tick(const std::string&, std::int64_t);
    MavlinkSessionSnapshot reconnect(const std::string&, const std::string&,
        const std::string&, std::uint8_t, std::uint8_t);
    std::optional<MavlinkSessionSnapshot> snapshot(const std::string&) const;
private:
    struct SessionState { MavlinkSessionSnapshot snapshot; };
    static std::optional<std::uint8_t> frameSequence(const std::vector<std::uint8_t>&);
    MavlinkSessionIngestResult ingestDecoded(SessionState&, std::uint8_t,
        std::int64_t, DecodedMavlinkMessage);
    std::int64_t heartbeat_timeout_ms_;
    std::unordered_map<std::string, SessionState> sessions_;
};
}
