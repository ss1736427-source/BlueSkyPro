#pragma once

#include "mavlink_telemetry_decoder_boundary.hpp"

#include <vector>

namespace bluesky::operations {

class MavlinkTelemetryReplayAssembler final {
public:
    static MavlinkTelemetryReplay assemble(
        const std::string& replay_id,
        const std::vector<DecodedMavlinkMessage>& messages);
};

} // namespace bluesky::operations
