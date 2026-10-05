#pragma once

#include "mavlink_telemetry_decoder_boundary.hpp"

#include <string>
#include <vector>

namespace bluesky::operations {

struct MavlinkRecordedStream {
    std::string stream_id;
    std::string format_version;
    std::vector<DecodedMavlinkMessage> messages;
};

class MavlinkRecordedStreamReader final {
public:
    static MavlinkRecordedStream read(const std::string& stream_id,
                                      const std::string& fixture);
};

} // namespace bluesky::operations
