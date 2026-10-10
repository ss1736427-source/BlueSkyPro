#include "px4_mavlink2_reference_adapter.hpp"
#include <utility>

namespace bluesky::autopilot {

Px4Mavlink2ReferenceAdapter::Px4Mavlink2ReferenceAdapter(Identity identity)
    : AutopilotAdapterBaseline(std::move(identity)) {}

Identity Px4Mavlink2ReferenceAdapter::getVehicleIdentity() const {
    auto identity = AutopilotAdapterBaseline::getVehicleIdentity();
    identity.autopilot = "PX4";
    identity.protocol = "MAVLINK2";
    return identity;
}

std::string Px4Mavlink2ReferenceAdapter::getAutopilotIdentity() const {
    return "PX4";
}

std::string Px4Mavlink2ReferenceAdapter::getProtocolVersion() const {
    return "MAVLINK2";
}

Capabilities Px4Mavlink2ReferenceAdapter::getCapabilities() const {
    return {
        {"arm", "disarm", "takeoff", "land", "hold", "resume", "rtl", "goto", "startMission", "pauseMission", "resumeMission", "abortMission"},
        {"MISSION_UPLOAD", "MISSION_READBACK", "SEMANTIC_VERIFY"},
        {"NORMALIZED_TELEMETRY", "FLIGHT_STATE", "LOG_ACQUISITION"}
    };
}

} // namespace bluesky::autopilot
