#include "mavlink_raw_decoder_boundary.hpp"

#ifdef NDEBUG
#undef NDEBUG
#endif

#include <cassert>
#include <iostream>
#include <cstring>

using namespace bluesky::operations;

static void crcAccumulate(std::uint8_t data, std::uint16_t& crc) {
    const auto tmp = static_cast<std::uint8_t>(data ^ static_cast<std::uint8_t>(crc & 0xffU));
    const auto tmp2 = static_cast<std::uint8_t>(tmp ^ static_cast<std::uint8_t>(tmp << 4));
    crc = static_cast<std::uint16_t>((crc >> 8) ^ (static_cast<std::uint16_t>(tmp2) << 8) ^ (static_cast<std::uint16_t>(tmp2) << 3) ^ (static_cast<std::uint16_t>(tmp2) >> 4));
}

static std::vector<std::uint8_t> makeFrame(
    std::uint32_t message_id, const std::vector<std::uint8_t>& payload,
    std::uint8_t crc_extra) {
    std::vector<std::uint8_t> frame{
        0xFD, static_cast<std::uint8_t>(payload.size()), 0, 0, 7,
        1, 1, static_cast<std::uint8_t>(message_id & 0xffU),
        static_cast<std::uint8_t>((message_id >> 8) & 0xffU),
        static_cast<std::uint8_t>((message_id >> 16) & 0xffU)};
    frame.insert(frame.end(), payload.begin(), payload.end());
    frame.push_back(0);
    frame.push_back(0);
    std::uint16_t crc = 0xffffU;
    for (std::size_t i = 1; i < frame.size() - 2; ++i) crcAccumulate(frame[i], crc);
    crcAccumulate(crc_extra, crc);
    frame[frame.size() - 2] = static_cast<std::uint8_t>(crc & 0xffU);
    frame[frame.size() - 1] = static_cast<std::uint8_t>(crc >> 8);
    return frame;
}

static void putU16(std::vector<std::uint8_t>& p, std::size_t offset, std::uint16_t value) {
    p[offset] = static_cast<std::uint8_t>(value & 0xffU);
    p[offset + 1] = static_cast<std::uint8_t>(value >> 8);
}

static void putU32(std::vector<std::uint8_t>& p, std::size_t offset, std::uint32_t value) {
    for (std::size_t i = 0; i < 4; ++i)
        p[offset + i] = static_cast<std::uint8_t>((value >> (8U * i)) & 0xffU);
}

static void putF32(std::vector<std::uint8_t>& p, std::size_t offset, float value) {
    std::uint32_t raw{};
    std::memcpy(&raw, &value, sizeof(raw));
    putU32(p, offset, raw);
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

void testScaledPressureDecodedWithBootTimeAndAbsolutePressure() {
    std::vector<std::uint8_t> payload(14, 0);
    putU32(payload, 0, 5000);
    putF32(payload, 4, 1013.25F);
    putF32(payload, 8, 0.0F);
    putU16(payload, 12, 1500);
    const auto frame = makeFrame(29, payload, 115);
    const auto decoded = MavlinkRawDecoderBoundary::decodeFrame(
        MavlinkDialect::PX4, "PX-UAV-028", "PX4:MAVLink2:1:1", 6000, frame);
    assert(decoded.has_value());
    assert(decoded->kind == MavlinkMessageKind::ScaledPressure);
    assert(decoded->timestamp_is_receive_time_fallback);
    assert(decoded->source_time_boot_ms == 5000);
    assert(decoded->barometric_pressure.has_value());
    assert(decoded->barometric_pressure->valid);
    assert(decoded->barometric_pressure->absolute_pressure_hpa == 1013.25);
    assert(decoded->barometric_pressure->received_timestamp_ms == 6000);
}

void testDistanceSensorDecodedWithoutAssumingItIsRadioAltimeter() {
    std::vector<std::uint8_t> payload(14, 0);
    putU32(payload, 0, 7000);
    putU16(payload, 4, 30);
    putU16(payload, 6, 10000);
    putU16(payload, 8, 3750);
    payload[10] = 3;  // radar sensor type
    payload[11] = 4;  // sensor instance ID
    payload[12] = 25; // downward-facing MAV_SENSOR_ORIENTATION
    const auto frame = makeFrame(132, payload, 85);
    const auto decoded = MavlinkRawDecoderBoundary::decodeFrame(
        MavlinkDialect::ArduPilot, "AP-UAV-028", "ArduPilot:MAVLink2:1:1", 8000, frame);
    assert(decoded.has_value());
    assert(decoded->kind == MavlinkMessageKind::DistanceSensor);
    assert(decoded->source_time_boot_ms == 7000);
    assert(decoded->distance_sensor.has_value());
    assert(decoded->distance_sensor->valid);
    assert(decoded->distance_sensor->sensor_id == 4);
    assert(decoded->distance_sensor->orientation == 25);
    assert(decoded->distance_sensor->distance_m.has_value());
    assert(*decoded->distance_sensor->distance_m == 37.5);
}

void testUnknownDistanceSentinelIsNotPublishedAsValidDistance() {
    std::vector<std::uint8_t> payload(14, 0);
    putU32(payload, 0, 7000);
    putU16(payload, 4, 30);
    putU16(payload, 6, 1000);
    putU16(payload, 8, 0xffffU);
    payload[10] = 3;
    payload[11] = 4;
    payload[12] = 25;
    const auto frame = makeFrame(132, payload, 85);
    const auto decoded = MavlinkRawDecoderBoundary::decodeFrame(
        MavlinkDialect::PX4, "PX-UAV-029", "PX4:MAVLink2:1:1", 8000, frame);
    assert(decoded.has_value());
    assert(decoded->distance_sensor.has_value());
    assert(!decoded->distance_sensor->valid);
    assert(!decoded->distance_sensor->distance_m.has_value());
}

int main() {
    testRawHeartbeatDecode();
    testCorruptFrameRejected();
    testUnsupportedMessageRejected();
    testScaledPressureDecodedWithBootTimeAndAbsolutePressure();
    testDistanceSensorDecodedWithoutAssumingItIsRadioAltimeter();
    testUnknownDistanceSentinelIsNotPublishedAsValidDistance();
    std::cout << "mavlink_raw_decoder_boundary_test: PASS\n";
    return 0;
}
