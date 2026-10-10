#pragma once

#include "model/mission_model.hpp"
#include "model/vehicle_equipment_capability.hpp"
#include "../AUTOPILOT_ADAPTER/core/universal_autopilot_adapter.hpp"

#include <string>

namespace bluesky::planning {

enum class MissionTransferState {
    Rejected,
    Compiled,
    Uploaded,
    ReadBack,
    Verified,
    Failed
};

struct MissionTransferResult {
    MissionTransferState state{MissionTransferState::Rejected};
    std::string mission_id;
    std::string vehicle_id;
    std::string compiled_mission;
    std::string readback_mission;
    std::string reason;
};

class MissionTransferRuntime final {
public:
    static MissionTransferResult execute(
        const Mission& mission,
        const VehicleEquipmentCapabilityProfile& capabilities,
        bluesky::autopilot::UniversalAutopilotAdapter& adapter);
};

} // namespace bluesky::planning
