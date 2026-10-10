#pragma once

#include "../../PLANNING/capability_execution_integration.hpp"
#include "operational_orchestrator_pipeline.hpp"

#include <string>
#include <vector>

namespace bluesky::operations {

enum class ActionExecutionKind {
    MissionTransfer,
    CommandDispatch
};

enum class ActionExecutionRouteState {
    Rejected,
    Routed
};

struct ActionExecutionBinding {
    std::string action_id;
    std::string device_id;
    ActionExecutionKind kind{ActionExecutionKind::CommandDispatch};
    std::vector<std::string> required_capabilities;
};

struct ActionExecutionRouteResult {
    ActionExecutionRouteState state{ActionExecutionRouteState::Rejected};
    std::string action_id;
    std::string reason;
    bluesky::planning::CapabilityExecutionMissionResult mission{};
    bluesky::planning::CapabilityExecutionCommandResult command{};
};

class OperationalActionExecutionRouter final {
public:
    static ActionExecutionRouteResult routeMission(
        const OrchestratorPipelineResult& decision,
        const ActionExecutionBinding& binding,
        const bluesky::planning::Mission& mission,
        const bluesky::planning::VehicleEquipmentCapabilityProfile& capabilities,
        bluesky::autopilot::UniversalAutopilotAdapter& adapter,
        CapabilityAdmissionBoundary& admission_boundary);

    static ActionExecutionRouteResult routeCommand(
        const OrchestratorPipelineResult& decision,
        const ActionExecutionBinding& binding,
        const bluesky::planning::command::CommandRequest& request,
        bluesky::planning::command::CommandLifecycleRuntime& lifecycle,
        CapabilityAdmissionBoundary& admission_boundary);
};

} // namespace bluesky::operations
