#pragma once

#include "mavlink_telemetry_decoder_boundary.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace bluesky::operations {

class MavlinkRawDecoderBoundary final {
public:
    static std::optional<DecodedMavlinkMessage> decodeFrame(
        MavlinkDialect dialect,
        const std::string& vehicle_id,
        const std::string& source_id,
        std::int64_t received_timestamp_ms,
        const std::vector<std::uint8_t>& frame);
};

} // namespace bluesky::operations
