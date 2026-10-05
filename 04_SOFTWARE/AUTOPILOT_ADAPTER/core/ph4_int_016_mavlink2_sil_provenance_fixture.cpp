#include "ph4_int_014_sil_execution_source.hpp"
#include "ph4_int_015_mavlink2_adapter.hpp"

#include <cassert>
#include <iostream>

int main() {
    using namespace bluesky::autopilot;

    const std::string flightRecordId = "FLIGHT-RECORD-PH4-INT-016";
    Mavlink2Adapter adapter({"UAV-MAV-016", "MULTIROTOR", "MAVLINK2-BASELINE", "SIL", "MAVLink2", "2"});
    SilExecutionSource source(flightRecordId);

    assert(adapter.connect());
    const auto result = adapter.startMission();
    assert(result.acknowledged);

    const auto frame = adapter.encodeCommand(result);
    const auto decoded = adapter.decodeCommandAck(frame);
    assert(decoded.has_value());
    assert(decoded->commandId == result.commandId);

    const auto event = source.emit(decoded->vehicleId, decoded->commandId, "ACKNOWLEDGED");
    assert(event.flightRecordId == flightRecordId);
    assert(event.vehicleId == "UAV-MAV-016");
    assert(event.source == "SIL_EXECUTION_SOURCE");
    assert(event.timestampMs > 0);

    std::cout << "PH4_INT_016|" << event.timestampMs << "|"
              << event.flightRecordId << "|" << event.vehicleId << "|"
              << event.commandId << "|" << event.executionState << "|"
              << Mavlink2Adapter::kProtocol << "|"
              << Mavlink2Adapter::kProtocolVersion << "|"
              << frame << "\n";
    return 0;
}
