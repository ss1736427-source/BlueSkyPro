#include "ph4_int_014_sil_execution_source.hpp"
#include "ph4_int_015_mavlink2_adapter.hpp"

#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {
void require(bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}
}

int main() {
    try {
        using namespace bluesky::autopilot;

        const std::string flightRecordId = "FLIGHT-RECORD-PH4-INT-017";
        const std::string approvedMission = "MISSION-017:UAV-MAV-017";

        Mavlink2Adapter adapter({"UAV-MAV-017", "MULTIROTOR", "MAVLINK2-BASELINE", "SIL", "MAVLink2", "2"});
        SilExecutionSource source(flightRecordId);

        require(adapter.connect(), "CONNECT_FAILED");
        require(adapter.uploadMission(approvedMission), "MISSION_UPLOAD_FAILED");

        const auto started = adapter.startMission();
        require(started.acknowledged, "MISSION_START_NOT_ACKNOWLEDGED");
        const auto startFrame = adapter.encodeCommand(started);
        require(!startFrame.empty(), "COMMAND_FRAME_EMPTY");
        const auto startAck = adapter.decodeCommandAck(startFrame);
        require(startAck.has_value(), "COMMAND_ACK_DECODE_FAILED");
        require(startAck->commandId == started.commandId, "COMMAND_ID_CORRELATION_FAILED");
        require(startAck->vehicleId == started.vehicleId, "VEHICLE_ID_CORRELATION_FAILED");

        const auto executionEvent = source.emit(startAck->vehicleId, startAck->commandId, "EXECUTING");
        require(executionEvent.flightRecordId == flightRecordId, "EXECUTING_FLIGHT_RECORD_CORRELATION_FAILED");
        require(executionEvent.vehicleId == startAck->vehicleId, "EXECUTING_VEHICLE_CORRELATION_FAILED");
        require(executionEvent.commandId == startAck->commandId, "EXECUTING_COMMAND_CORRELATION_FAILED");

        const auto readback = adapter.readBackMission();
        require(readback.has_value(), "MISSION_READBACK_EMPTY");
        require(*readback == approvedMission, "MISSION_READBACK_CONTENT_FAILED");

        const auto readbackFrame = adapter.encodeMissionReadback(*readback);
        require(!readbackFrame.empty(), "MISSION_READBACK_FRAME_EMPTY");
        const auto decodedReadback = adapter.decodeMissionReadback(readbackFrame);
        require(decodedReadback.has_value(), "MISSION_READBACK_DECODE_FAILED");
        require(*decodedReadback == approvedMission, "MISSION_READBACK_DECODE_CONTENT_FAILED");

        const auto comparison = adapter.compareMission(approvedMission, *decodedReadback);
        require(comparison.equal, "MISSION_READBACK_COMPARISON_FAILED");

        const auto completedEvent = source.emit(startAck->vehicleId, startAck->commandId, "COMPLETED");
        require(completedEvent.flightRecordId == flightRecordId, "COMPLETED_FLIGHT_RECORD_CORRELATION_FAILED");
        require(completedEvent.vehicleId == startAck->vehicleId, "COMPLETED_VEHICLE_CORRELATION_FAILED");
        require(completedEvent.commandId == startAck->commandId, "COMPLETED_COMMAND_CORRELATION_FAILED");

        std::cout << "PH4_INT_017|" << flightRecordId << "|"
                  << startAck->vehicleId << "|" << startAck->commandId << "|"
                  << "EXECUTING" << "|" << "COMPLETED" << "|"
                  << Mavlink2Adapter::kProtocol << "|"
                  << Mavlink2Adapter::kProtocolVersion << "|"
                  << startFrame << "|" << readbackFrame << "|"
                  << approvedMission << "|" << *decodedReadback << "|"
                  << (comparison.equal ? "MATCH" : "MISMATCH") << "\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "PH4_INT_017_EXCEPTION|" << error.what() << "\n";
        return 1;
    } catch (...) {
        std::cerr << "PH4_INT_017_EXCEPTION|UNKNOWN\n";
        return 1;
    }
}
