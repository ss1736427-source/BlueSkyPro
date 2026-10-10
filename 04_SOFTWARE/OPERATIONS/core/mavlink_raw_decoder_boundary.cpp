#include "mavlink_raw_decoder_boundary.hpp"

#include <cmath>
#include <cstring>

namespace bluesky::operations {
namespace {
std::uint16_t readU16(const std::vector<std::uint8_t>& p, std::size_t o) { return static_cast<std::uint16_t>(p[o]) | (static_cast<std::uint16_t>(p[o + 1]) << 8); }
std::int16_t readI16(const std::vector<std::uint8_t>& p, std::size_t o) { return static_cast<std::int16_t>(readU16(p, o)); }
std::uint32_t readU32(const std::vector<std::uint8_t>& p, std::size_t o) { return static_cast<std::uint32_t>(p[o]) | (static_cast<std::uint32_t>(p[o + 1]) << 8) | (static_cast<std::uint32_t>(p[o + 2]) << 16) | (static_cast<std::uint32_t>(p[o + 3]) << 24); }
std::int32_t readI32(const std::vector<std::uint8_t>& p, std::size_t o) { return static_cast<std::int32_t>(readU32(p, o)); }
float readF32(const std::vector<std::uint8_t>& p, std::size_t o) { const auto raw = readU32(p, o); float value{}; std::memcpy(&value, &raw, sizeof(value)); return value; }
void crcAccumulate(std::uint8_t data, std::uint16_t& crc) { const auto tmp = static_cast<std::uint8_t>(data ^ static_cast<std::uint8_t>(crc & 0xffU)); const auto tmp2 = static_cast<std::uint8_t>(tmp ^ static_cast<std::uint8_t>(tmp << 4)); crc = static_cast<std::uint16_t>((crc >> 8) ^ (static_cast<std::uint16_t>(tmp2) << 8) ^ (static_cast<std::uint16_t>(tmp2) << 3) ^ (static_cast<std::uint16_t>(tmp2) >> 4)); }
std::uint16_t frameCrc(const std::vector<std::uint8_t>& frame, std::uint8_t extra) { std::uint16_t crc = 0xffffU; for (std::size_t i = 1; i + 2 < frame.size(); ++i) crcAccumulate(frame[i], crc); crcAccumulate(extra, crc); return crc; }
std::optional<std::uint8_t> crcExtra(std::uint32_t msgid) { switch (msgid) { case 0: return 50; case 1: return 124; case 30: return 39; case 33: return 104; case 111: return 34; default: return std::nullopt; } }
} // namespace

std::optional<DecodedMavlinkMessage> MavlinkRawDecoderBoundary::decodeFrame(MavlinkDialect dialect, const std::string& vehicle_id, const std::string& source_id, std::int64_t received_timestamp_ms, const std::vector<std::uint8_t>& frame) {
    if (frame.size() < 12 || frame[0] != 0xFDU || vehicle_id.empty() || source_id.empty() || received_timestamp_ms <= 0) return std::nullopt;
    const std::size_t payload_len = frame[1];
    if (frame.size() != 10U + payload_len + 2U || frame[5] == 0 || frame[6] == 0) return std::nullopt;
    const auto msgid = static_cast<std::uint32_t>(frame[7]) | (static_cast<std::uint32_t>(frame[8]) << 8) | (static_cast<std::uint32_t>(frame[9]) << 16);
    const auto extra = crcExtra(msgid);
    if (!extra.has_value()) return std::nullopt;
    const auto expected_crc = static_cast<std::uint16_t>(frame[10U + payload_len]) | (static_cast<std::uint16_t>(frame[11U + payload_len]) << 8);
    if (frameCrc(frame, *extra) != expected_crc) return std::nullopt;
    const std::vector<std::uint8_t> payload(frame.begin() + 10, frame.begin() + 10 + payload_len);
    DecodedMavlinkMessage message;
    message.dialect = dialect; message.vehicle_id = vehicle_id; message.source_id = source_id;
    message.system_id = frame[5]; message.component_id = frame[6]; message.received_timestamp_ms = received_timestamp_ms; message.valid = true;
    switch (msgid) {
    case 0:
        if (payload.size() < 9) return std::nullopt;
        message.kind = MavlinkMessageKind::Heartbeat; message.healthy = payload[7] != 0;
        message.flight_mode = std::string("BASE_MODE:") + std::to_string(payload[6]); break;
    case 33:
        if (payload.size() < 28) return std::nullopt;
        message.kind = MavlinkMessageKind::GlobalPositionInt;
        message.latitude_deg = static_cast<double>(readI32(payload, 4)) / 1e7; message.longitude_deg = static_cast<double>(readI32(payload, 8)) / 1e7; message.altitude_m = static_cast<double>(readI32(payload, 12)) / 1000.0;
        { const auto vx = static_cast<double>(readI16(payload, 20)) / 100.0; const auto vy = static_cast<double>(readI16(payload, 22)) / 100.0; message.ground_speed_mps = std::sqrt(vx * vx + vy * vy); }
        { const auto heading = readU16(payload, 26); if (heading != 0xffffU) message.heading_deg = static_cast<double>(heading) / 100.0; }
        break;
    case 30:
        if (payload.size() < 16) return std::nullopt;
        message.kind = MavlinkMessageKind::Attitude; message.roll_rad = static_cast<double>(readF32(payload, 4)); message.pitch_rad = static_cast<double>(readF32(payload, 8)); message.yaw_rad = static_cast<double>(readF32(payload, 12)); break;
    case 1:
        if (payload.size() < 31) return std::nullopt;
        message.kind = MavlinkMessageKind::SysStatus; if (static_cast<std::int8_t>(payload[30]) >= 0) message.battery_percent = static_cast<double>(static_cast<std::int8_t>(payload[30])); message.healthy = true; break;
    case 111:
        if (payload.size() < 16) return std::nullopt;
        message.kind = MavlinkMessageKind::Timesync; break;
    default: return std::nullopt;
    }
    return message;
}
} // namespace bluesky::operations
