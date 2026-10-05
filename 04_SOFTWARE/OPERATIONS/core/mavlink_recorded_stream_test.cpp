#include "mavlink_recorded_stream.hpp"
#include "mavlink_telemetry_replay_assembler.hpp"
#include "mavlink_telemetry_mapping.hpp"

#ifdef NDEBUG
#undef NDEBUG
#endif

#include <cassert>
#include <iostream>

using namespace bluesky::operations;

void testVersionedStreamIsRead() {
    const std::string fixture =
        "BLUESKY-MAVLINK-REC,1\n"
        "1 ArduPilot HEARTBEAT AP-UAV-026 ArduPilot:MAVLink2:1:1 1 1 1000 1000 1 1 AUTO\n"
        "1 ArduPilot GLOBAL_POSITION_INT AP-UAV-026 ArduPilot:MAVLink2:1:1 1 1 1100 1100 1 1 60.1 24.9 82 18 91\n";

    const auto stream = MavlinkRecordedStreamReader::read("AP-REC-026", fixture);
    assert(stream.stream_id == "AP-REC-026");
    assert(stream.format_version == "1");
    assert(stream.messages.size() == 2);
    assert(stream.messages[0].flight_mode.has_value());
    assert(stream.messages[1].latitude_deg.has_value());
    assert(stream.messages[1].longitude_deg.has_value());
    assert(stream.messages[1].source_timestamp_ms == 1100);
}

void testRecordedStreamFeedsExistingReplayPath() {
    const std::string fixture =
        "BLUESKY-MAVLINK-REC,1\n"
        "1 ArduPilot HEARTBEAT AP-UAV-026 ArduPilot:MAVLink2:1:1 1 1 1000 1000 1 1 AUTO\n"
        "1 ArduPilot GLOBAL_POSITION_INT AP-UAV-026 ArduPilot:MAVLink2:1:1 1 1 1100 1100 1 1 60.1 24.9 82 18 91\n"
        "1 ArduPilot ATTITUDE AP-UAV-026 ArduPilot:MAVLink2:1:1 1 1 1200 1200 1 1 0.01 -0.02 1.58\n"
        "1 ArduPilot SYS_STATUS AP-UAV-026 ArduPilot:MAVLink2:1:1 1 1 1300 1300 1 1 76\n";

    const auto stream = MavlinkRecordedStreamReader::read("AP-REC-026", fixture);
    const auto replay = MavlinkTelemetryReplayAssembler::assemble(stream.stream_id, stream.messages);
    assert(replay.samples.size() == 1);

    const auto state = MavlinkTelemetryMapping::replayLastUsable(replay);
    assert(normalized_state_usable(state));
    assert(state.vehicle_id == "AP-UAV-026");
    assert(state.provenance.source_id == "ArduPilot:MAVLink2:1:1");
}

void testUnknownVersionAndMessageAreIgnored() {
    const std::string fixture =
        "BLUESKY-MAVLINK-REC,1\n"
        "2 PX4 HEARTBEAT PX-UAV-026 PX4:MAVLink2:1:1 1 1 1000 1000 1 1 AUTO\n"
        "1 PX4 UNKNOWN PX-UAV-026 PX4:MAVLink2:1:1 1 1 1100 1100 1 1\n";

    const auto stream = MavlinkRecordedStreamReader::read("PX-REC-026", fixture);
    assert(stream.messages.empty());
}

int main() {
    testVersionedStreamIsRead();
    testRecordedStreamFeedsExistingReplayPath();
    testUnknownVersionAndMessageAreIgnored();
    std::cout << "mavlink_recorded_stream_test: PASS\n";
    return 0;
}
