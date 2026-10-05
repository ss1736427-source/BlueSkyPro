#include "ph4_int_014_sil_execution_source.hpp"

#include <chrono>
#include <stdexcept>

namespace bluesky::autopilot {

SilExecutionSource::SilExecutionSource(std::string flightRecordId)
    : flightRecordId_(std::move(flightRecordId)) {
    if (flightRecordId_.empty()) {
        throw std::invalid_argument("flight_record_id_required");
    }
}

SilExecutionEvent SilExecutionSource::emit(const std::string& vehicleId,
                                           const std::string& commandId,
                                           const std::string& executionState) {
    if (vehicleId.empty() || commandId.empty() || executionState.empty()) {
        throw std::invalid_argument("vehicle_command_state_required");
    }
    const auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    return SilExecutionEvent{
        static_cast<std::uint64_t>(now) + sequence_++,
        flightRecordId_, vehicleId, executionState, commandId, "SIL_EXECUTION_SOURCE"};
}

} // namespace bluesky::autopilot
