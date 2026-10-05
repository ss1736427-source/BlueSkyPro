#include "mavlink_recorded_stream.hpp"

#include <sstream>

namespace bluesky::operations {

MavlinkRecordedStream MavlinkRecordedStreamReader::read(
    const std::string& stream_id,
    const std::string& fixture) {
    MavlinkRecordedStream stream;
    stream.stream_id = stream_id;

    std::istringstream input(fixture);
    std::string line;
    bool header_seen = false;

    while (std::getline(input, line)) {
        if (line.empty() || line[0] == '#') continue;

        if (line.rfind("BLUESKY-MAVLINK-REC,", 0) == 0) {
            const auto comma = line.find(',');
            stream.format_version = line.substr(comma + 1);
            header_seen = true;
            continue;
        }
        if (!header_seen) continue;

        std::istringstream fields(line);
        std::string version, dialect, kind, vehicle_id, source_id;
        int system_id = 0, component_id = 0, healthy = 0, valid = 0;
        std::int64_t source_timestamp_ms = 0, received_timestamp_ms = 0;
        double v1 = 0.0, v2 = 0.0, v3 = 0.0, v4 = 0.0, v5 = 0.0;

        if (!(fields >> version >> dialect >> kind >> vehicle_id >> source_id
              >> system_id >> component_id >> source_timestamp_ms
              >> received_timestamp_ms >> healthy >> valid)) continue;
        if (version != stream.format_version) continue;

        DecodedMavlinkMessage message;
        message.dialect = dialect == "ArduPilot"
            ? MavlinkDialect::ArduPilot : MavlinkDialect::PX4;

        if (kind == "HEARTBEAT") message.kind = MavlinkMessageKind::Heartbeat;
        else if (kind == "GLOBAL_POSITION_INT") message.kind = MavlinkMessageKind::GlobalPositionInt;
        else if (kind == "ATTITUDE") message.kind = MavlinkMessageKind::Attitude;
        else if (kind == "SYS_STATUS") message.kind = MavlinkMessageKind::SysStatus;
        else if (kind == "MISSION_STATE") message.kind = MavlinkMessageKind::MissionState;
        else continue;

        message.vehicle_id = vehicle_id;
        message.source_id = source_id;
        message.system_id = static_cast<std::uint8_t>(system_id);
        message.component_id = static_cast<std::uint8_t>(component_id);
        message.source_timestamp_ms = source_timestamp_ms;
        message.received_timestamp_ms = received_timestamp_ms;
        message.healthy = healthy != 0;
        message.valid = valid != 0;

        if (message.kind == MavlinkMessageKind::Heartbeat) {
            std::string flight_mode;
            if (fields >> flight_mode) {
                message.flight_mode = flight_mode;
            }
        } else if (message.kind == MavlinkMessageKind::GlobalPositionInt &&
                   (fields >> v1 >> v2 >> v3 >> v4 >> v5)) {
            message.latitude_deg = v1;
            message.longitude_deg = v2;
            message.altitude_m = v3;
            message.ground_speed_mps = v4;
            message.heading_deg = v5;
        } else if (message.kind == MavlinkMessageKind::Attitude &&
                   (fields >> v1 >> v2 >> v3)) {
            message.roll_rad = v1;
            message.pitch_rad = v2;
            message.yaw_rad = v3;
        } else if (message.kind == MavlinkMessageKind::SysStatus &&
                   (fields >> v1)) {
            message.battery_percent = v1;
        }

        stream.messages.push_back(message);
    }

    return stream;
}

} // namespace bluesky::operations
