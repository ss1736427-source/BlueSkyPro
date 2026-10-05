#pragma once

#include "../OPERATIONS/core/capability_admission_boundary.hpp"
#include "command_lifecycle_runtime.hpp"
#include "mission_transfer_runtime.hpp"

#include <string>
#include <vector>

namespace bluesky::planning {

struct CapabilityExecutionMissionResult {
    MissionTransferResult transfer;
    operations::CapabilityAdmissionDecision admission;
};

struct CapabilityExecutionCommandResult {
    command::CommandTransition transition;
    operations::CapabilityAdmissionDecision admission;
};

class CapabilityExecutionIntegration final {
public:
    static CapabilityExecutionMissionResult executeMission(
        operations::CapabilityAdmissionBoundary& admission_boundary,
        const std::string& device_id,
        const Mission& mission,
        const std::vector<std::string>& required_capabilities,
        const VehicleEquipmentCapabilityProfile& capabilities,
        bluesky::autopilot::UniversalAutopilotAdapter& adapter);

    static CapabilityExecutionCommandResult dispatchCommand(
        operations::CapabilityAdmissionBoundary& admission_boundary,
        const std::string& device_id,
        const command::CommandRequest& request,
        const std::vector<std::string>& required_capabilities,
        command::CommandLifecycleRuntime& lifecycle);
};

} // namespace bluesky::planning
