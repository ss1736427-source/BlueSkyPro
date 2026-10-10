#include "ph4_int_015_mavlink2_adapter.hpp"

#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {
void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}
}

int main() {
    try {
        using namespace bluesky::autopilot;
        const std::string flightRecordId = "FLIGHT-RECORD-PH4-INT-019";
        const std::string configuration = "CFG-019:MISSION-SAFE-PARAMETERS";
        const std::string wrongConfiguration = "CFG-019:UNVERIFIED-PARAMETERS";

        Mavlink2Adapter adapter({"UAV-MAV-019", "MULTIROTOR", "MAVLINK2-BASELINE", "SIL", "MAVLink2", "2"});
        require(adapter.connect(), "CONNECT_FAILED");
        const auto initialBaseline = adapter.readBaseline();
        require(!initialBaseline.has_value(), "INITIAL_BASELINE_SHOULD_BE_EMPTY");

        const auto emptyValidation = adapter.validate("");
        require(!emptyValidation.acknowledged, "EMPTY_CONFIGURATION_ACCEPTED");
        require(emptyValidation.error == ErrorCode::InvalidState, "EMPTY_CONFIGURATION_ERROR_FAILED");

        const auto writeResult = adapter.write(configuration);
        require(writeResult.acknowledged, "CONFIGURATION_WRITE_FAILED");
        require(writeResult.vehicleId == "UAV-MAV-019", "WRITE_VEHICLE_CORRELATION_FAILED");

        const auto readback = adapter.readBack();
        require(readback.has_value(), "CONFIGURATION_READBACK_EMPTY");
        require(*readback == configuration, "CONFIGURATION_READBACK_CONTENT_FAILED");

        const auto comparison = adapter.compare(configuration, *readback);
        require(comparison.equal, "CONFIGURATION_COMPARISON_FAILED");

        const auto verifyResult = adapter.verify(configuration);
        require(verifyResult.acknowledged, "CONFIGURATION_VERIFY_FAILED");
        require(verifyResult.executionState == ExecutionState::Completed, "CONFIGURATION_VERIFY_STATE_FAILED");
        require(verifyResult.error == ErrorCode::None, "CONFIGURATION_VERIFY_ERROR_FAILED");

        const auto mismatch = adapter.compare(configuration, wrongConfiguration);
        require(!mismatch.equal, "CONFIGURATION_MISMATCH_NOT_DETECTED");
        require(!mismatch.mismatches.empty(), "CONFIGURATION_MISMATCH_DETAILS_EMPTY");

        const auto failedVerify = adapter.verify(wrongConfiguration);
        require(!failedVerify.acknowledged, "WRONG_CONFIGURATION_VERIFIED");
        require(failedVerify.executionState == ExecutionState::Failed, "FAILED_VERIFY_STATE_FAILED");
        require(failedVerify.error == ErrorCode::ConfigurationMismatch, "FAILED_VERIFY_ERROR_FAILED");

        std::cout << "PH4_INT_019|" << flightRecordId << "|UAV-MAV-019|"
                  << Mavlink2Adapter::kProtocol << "|" << Mavlink2Adapter::kProtocolVersion << "|"
                  << "BASELINE_EMPTY|VALIDATION_REJECTED|WRITE|READBACK|MATCH|VERIFIED|MISMATCH|VERIFY_FAILED\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "PH4_INT_019_EXCEPTION|" << error.what() << "\n";
        return 1;
    } catch (...) {
        std::cerr << "PH4_INT_019_EXCEPTION|UNKNOWN\n";
        return 1;
    }
}
