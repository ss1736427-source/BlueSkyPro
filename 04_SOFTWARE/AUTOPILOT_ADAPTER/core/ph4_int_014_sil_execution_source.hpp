#pragma once

#include <cstdint>
#include <string>

namespace bluesky::autopilot {

struct SilExecutionEvent {
    std::uint64_t timestampMs{0};
    std::string flightRecordId;
    std::string vehicleId;
    std::string executionState;
    std::string commandId;
    std::string source;
};

class SilExecutionSource {
public:
    explicit SilExecutionSource(std::string flightRecordId);

    SilExecutionEvent emit(const std::string& vehicleId,
                           const std::string& commandId,
                           const std::string& executionState);

private:
    std::string flightRecordId_;
    std::uint64_t sequence_{0};
};

} // namespace bluesky::autopilot
