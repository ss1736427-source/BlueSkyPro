#pragma once

#include "normalized_telemetry_state.hpp"
#include "../../AUTOPILOT_ADAPTER/core/universal_autopilot_adapter.hpp"

#include <cstdint>
#include <string>

namespace bluesky::operations {

struct VehicleStateTelemetryFrame {
    std::string vehicle_id;
    std::string source_id;
    std::string source_type{"autopilot"};
    std::string adapter_version;
    std::string snapshot_version{"vehicle-state-1"};
    std::int64_t source_timestamp_ms{0};
    std::int64_t received_timestamp_ms{0};
    bool position_valid{false};
    bool navigation_valid{false};
    bool vehicle_state_valid{false};
    bool energy_state_valid{false};
    bool c2_state_valid{false};
    bool health_valid{false};
    bool mission_valid{false};
};

class VehicleStateMapping final {
public:
    static NormalizedTelemetryState fromAdapter(
        const bluesky::autopilot::UniversalAutopilotAdapter& adapter,
        std::int64_t source_timestamp_ms,
        std::int64_t received_timestamp_ms);

    static NormalizedTelemetryState fromFrame(
        const VehicleStateTelemetryFrame& frame);

private:
    static TelemetryFreshness classifyFreshness(
        std::int64_t source_timestamp_ms,
        std::int64_t received_timestamp_ms);
};

} // namespace bluesky::operations
