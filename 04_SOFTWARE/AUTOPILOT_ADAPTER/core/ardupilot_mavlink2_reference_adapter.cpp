#include "ardupilot_mavlink2_reference_adapter.hpp"
#include <utility>

namespace bluesky::autopilot {

ArduPilotMavlink2ReferenceAdapter::ArduPilotMavlink2ReferenceAdapter(Identity identity)
    : AutopilotAdapterBaseline(std::move(identity)) {}

Identity ArduPilotMavlink2ReferenceAdapter::getVehicleIdentity() const {
    auto identity = AutopilotAdapterBaseline::getVehicleIdentity();
    identity.autopilot = "ArduPilot";
    identity.protocol = "MAVLINK2";
    return identity;
}

std::string ArduPilotMavlink2ReferenceAdapter::getAutopilotIdentity() const {
    return "ArduPilot";
}

std::string ArduPilotMavlink2ReferenceAdapter::getProtocolVersion() const {
    return "MAVLINK2";
}

Capabilities ArduPilotMavlink2ReferenceAdapter::getCapabilities() const {
    return {
        {"arm", "disarm", "takeoff", "land", "hold", "resume", "rtl", "goto", "startMission", "pauseMission", "resumeMission", "abortMission"},
        {"MISSION_UPLOAD", "MISSION_READBACK", "SEMANTIC_VERIFY"},
        {"NORMALIZED_TELEMETRY", "FLIGHT_STATE", "LOG_ACQUISITION"}
    };
}

} // namespace bluesky::autopilot
