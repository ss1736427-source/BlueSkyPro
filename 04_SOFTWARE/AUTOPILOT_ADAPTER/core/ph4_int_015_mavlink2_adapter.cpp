#include "ph4_int_015_mavlink2_adapter.hpp"

#include <sstream>
#include <stdexcept>
#include <utility>

namespace bluesky::autopilot {

Mavlink2Adapter::Mavlink2Adapter(Identity identity)
    : AutopilotAdapterBaseline(std::move(identity)) {
    if (getVehicleIdentity().protocol != kProtocol ||
        getVehicleIdentity().protocolVersion != kProtocolVersion) {
        throw std::invalid_argument("MAVLink2_PROTOCOL_IDENTITY_REQUIRED");
    }
}

std::string Mavlink2Adapter::encodeCommand(const CommandResult& result) const {
    if (result.commandId.empty() || result.vehicleId.empty()) {
        throw std::invalid_argument("COMMAND_RESULT_IDENTITY_REQUIRED");
    }
    std::ostringstream out;
    out << "MAVLINK2|CMD_ACK|" << result.commandId << "|"
        << result.vehicleId << "|" << static_cast<int>(result.executionState)
        << "|" << (result.acknowledged ? "1" : "0") << "|"
        << static_cast<int>(result.error);
    return out.str();
}

std::optional<CommandResult> Mavlink2Adapter::decodeCommandAck(const std::string& frame) const {
    std::stringstream in(frame);
    std::string tag, type, commandId, vehicleId, state, ack, error;
    if (!std::getline(in, tag, '|') || !std::getline(in, type, '|') ||
        !std::getline(in, commandId, '|') || !std::getline(in, vehicleId, '|') ||
        !std::getline(in, state, '|') || !std::getline(in, ack, '|') ||
        !std::getline(in, error, '|') || tag != "MAVLINK2" || type != "CMD_ACK") {
        return std::nullopt;
    }
    try {
        CommandResult result;
        result.commandId = commandId;
        result.vehicleId = vehicleId;
        result.executionState = static_cast<ExecutionState>(std::stoi(state));
        result.acknowledged = ack == "1";
        result.error = static_cast<ErrorCode>(std::stoi(error));
        result.reason = "MAVLINK2_ACK_DECODED";
        return result;
    } catch (...) {
        return std::nullopt;
    }
}

std::string Mavlink2Adapter::encodeMissionReadback(const std::string& mission) const {
    if (mission.empty()) return {};
    return "MAVLINK2|MISSION_READBACK|" + mission;
}

std::optional<std::string> Mavlink2Adapter::decodeMissionReadback(const std::string& frame) const {
    const std::string prefix = "MAVLINK2|MISSION_READBACK|";
    if (frame.rfind(prefix, 0) != 0 || frame.size() == prefix.size()) return std::nullopt;
    return frame.substr(prefix.size());
}

} // namespace bluesky::autopilot
