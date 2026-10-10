#include "ph4_int_015_mavlink2_adapter.hpp"

#include <cassert>

int main() {
    using namespace bluesky::autopilot;

    Mavlink2Adapter adapter({"UAV-MAV-015", "MULTIROTOR", "MAVLINK2-BASELINE", "FIXTURE", "MAVLink2", "2"});
    assert(adapter.getVehicleIdentity().protocol == "MAVLink2");
    assert(adapter.getProtocolVersion() == "2");
    assert(adapter.connect());

    const auto command = adapter.startMission();
    assert(command.acknowledged);
    const auto frame = adapter.encodeCommand(command);
    const auto decoded = adapter.decodeCommandAck(frame);
    assert(decoded.has_value());
    assert(decoded->commandId == command.commandId);
    assert(decoded->vehicleId == command.vehicleId);
    assert(decoded->acknowledged);

    const auto missionFrame = adapter.encodeMissionReadback("MISSION-015");
    const auto mission = adapter.decodeMissionReadback(missionFrame);
    assert(mission.has_value());
    assert(*mission == "MISSION-015");

    assert(!adapter.decodeCommandAck("INVALID|FRAME"));
    assert(!adapter.decodeMissionReadback("MAVLINK2|MISSION_READBACK|"));
    return 0;
}
