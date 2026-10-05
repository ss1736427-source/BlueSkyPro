#include "mavlink_raw_decoder_boundary.hpp"

#ifdef NDEBUG
#undef NDEBUG
#endif

#include <cassert>
#include <iostream>

using namespace bluesky::operations;

static void crcAccumulate(std::uint8_t data, std::uint16_t& crc) {
    const auto tmp = static_cast<std::uint8_t>(data ^ static_cast<std::uint8_t>(crc & 0xffU));
    const auto tmp2 = static_cast<std::uint8_t>(tmp ^ static_cast<std::uint8_t>(tmp << 4));
    crc = static_cast<std::uint16_t>((crc >> 8) ^ (static_cast<std::uint16_t>(tmp2) << 8) ^ (static_cast<std::uint16_t>(tmp2) << 3) ^ (static_cast<std::uint16_t>(tmp2) >> 4));
}

static std::vector<std::uint8_t> heartbeatFrame() {
    std::vector<std::uint8_t> frame{0xFD, 9, 0, 0, 7, 1, 1, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0};
    std::uint16_t crc = 0xffffU;
    for (std::size_t i = 1; i < frame.size() - 2; ++i) crcAccumulate(frame[i], crc);
    crcAccumulate(50, crc);
    frame[frame.size() - 2] = static_cast<std::uint8_t>(crc & 0xffU);
    frame[frame.size() - 1] = static_cast<std::uint8_t>(crc >> 8);
    return frame;
}

void testRawHeartbeatDecode() {
    const auto decoded = MavlinkRawDecoderBoundary::decodeFrame(
        MavlinkDialect::ArduPilot, "AP-UAV-027", "ArduPilot:MAVLink2:1:1", 2000, heartbeatFrame());
    assert(decoded.has_value());
    assert(decoded->kind == MavlinkMessageKind::Heartbeat);
    assert(decoded->system_id == 1);
    assert(decoded->component_id == 1);
    assert(decoded->healthy);
    assert(decoded->flight_mode.has_value());
    assert(decoded->valid);
}

void testCorruptFrameRejected() {
    auto frame = heartbeatFrame();
    frame[12] ^= 0x01U;
    const auto decoded = MavlinkRawDecoderBoundary::decodeFrame(
        MavlinkDialect::PX4, "PX-UAV-027", "PX4:MAVLink2:1:1", 2000, frame);
    assert(!decoded.has_value());
}

void testUnsupportedMessageRejected() {
    auto frame = heartbeatFrame();
    frame[7] = 150;
    const auto decoded = MavlinkRawDecoderBoundary::decodeFrame(
        MavlinkDialect::ArduPilot, "AP-UAV-027", "ArduPilot:MAVLink2:1:1", 2000, frame);
    assert(!decoded.has_value());
}

int main() {
    testRawHeartbeatDecode();
    testCorruptFrameRejected();
    testUnsupportedMessageRejected();
    std::cout << "mavlink_raw_decoder_boundary_test: PASS\n";
    return 0;
}
