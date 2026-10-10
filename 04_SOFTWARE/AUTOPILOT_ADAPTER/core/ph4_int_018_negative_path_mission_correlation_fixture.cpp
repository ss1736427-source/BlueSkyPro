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

        const std::string flightRecordId = "FLIGHT-RECORD-PH4-INT-018";
        const std::string approvedMission = "MISSION-018:UAV-MAV-018";
        const std::string mismatchedMission = "MISSION-018:UAV-MAV-018-MISMATCH";

        Mavlink2Adapter adapter({"UAV-MAV-018", "MULTIROTOR", "MAVLINK2-BASELINE", "SIL", "MAVLink2", "2"});
        SilExecutionSource source(flightRecordId);

        require(adapter.connect(), "CONNECT_FAILED");
        require(adapter.uploadMission(mismatchedMission), "MISSION_UPLOAD_FAILED");

        const auto started = adapter.startMission();
        require(started.acknowledged, "MISSION_START_NOT_ACKNOWLEDGED");
        const auto startFrame = adapter.encodeCommand(started);
        require(!startFrame.empty(), "COMMAND_FRAME_EMPTY");
        const auto startAck = adapter.decodeCommandAck(startFrame);
        require(startAck.has_value(), "COMMAND_ACK_DECODE_FAILED");
        require(startAck->commandId == started.commandId, "COMMAND_ID_CORRELATION_FAILED");
        require(startAck->vehicleId == started.vehicleId, "VEHICLE_ID_CORRELATION_FAILED");

        const auto executing = source.emit(startAck->vehicleId, startAck->commandId, "EXECUTING");
        require(executing.flightRecordId == flightRecordId, "EXECUTING_FLIGHT_RECORD_CORRELATION_FAILED");
        require(executing.vehicleId == startAck->vehicleId, "EXECUTING_VEHICLE_CORRELATION_FAILED");
        require(executing.commandId == startAck->commandId, "EXECUTING_COMMAND_CORRELATION_FAILED");

        const auto readback = adapter.readBackMission();
        require(readback.has_value(), "MISSION_READBACK_EMPTY");
        const std::string actualMission = readback.value();

        const auto readbackFrame = adapter.encodeMissionReadback(actualMission);
        require(!readbackFrame.empty(), "MISSION_READBACK_FRAME_EMPTY");
        const auto decodedReadback = adapter.decodeMissionReadback(readbackFrame);
        require(decodedReadback.has_value(), "MISSION_READBACK_DECODE_FAILED");
        const std::string decodedMission = decodedReadback.value();

        const auto comparison = adapter.compareMission(approvedMission, decodedMission);
        require(!comparison.equal, "MISSION_MISMATCH_NOT_DETECTED");
        require(!comparison.mismatches.empty(), "MISSION_MISMATCH_DETAILS_EMPTY");

        const auto failed = source.emit(startAck->vehicleId, startAck->commandId, "FAILED");
        require(failed.flightRecordId == flightRecordId, "FAILED_FLIGHT_RECORD_CORRELATION_FAILED");
        require(failed.vehicleId == executing.vehicleId, "FAILED_VEHICLE_CORRELATION_FAILED");
        require(failed.commandId == executing.commandId, "FAILED_COMMAND_CORRELATION_FAILED");

        std::cout << "PH4_INT_018|" << flightRecordId << "|"
                  << startAck->vehicleId << "|" << startAck->commandId << "|"
                  << executing.executionState << "|" << failed.executionState << "|"
                  << Mavlink2Adapter::kProtocol << "|"
                  << Mavlink2Adapter::kProtocolVersion << "|"
                  << startFrame << "|" << readbackFrame << "|"
                  << approvedMission << "|" << decodedMission << "|"
                  << "MISMATCH" << "|" << "EXECUTION_FAILURE" << "\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "PH4_INT_018_EXCEPTION|" << error.what() << "\n";
        return 1;
    } catch (...) {
        std::cerr << "PH4_INT_018_EXCEPTION|UNKNOWN\n";
        return 1;
    }
}
