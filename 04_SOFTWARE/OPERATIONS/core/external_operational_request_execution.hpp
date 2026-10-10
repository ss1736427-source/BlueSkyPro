#pragma once

#include "external_operational_request_intake.hpp"

namespace bluesky::operations {

class ExternalOperationalRequestExecution final {
public:
    static ActionExecutionRouteResult executeMission(
        const ExternalOperationalRequestResult& accepted_request,
        const bluesky::planning::Mission& mission,
        const bluesky::planning::VehicleEquipmentCapabilityProfile& capabilities,
        bluesky::autopilot::UniversalAutopilotAdapter& adapter,
        CapabilityAdmissionBoundary& admission_boundary);

    static ActionExecutionRouteResult executeCommand(
        const ExternalOperationalRequestResult& accepted_request,
        const bluesky::planning::command::CommandRequest& request,
        bluesky::planning::command::CommandLifecycleRuntime& lifecycle,
        CapabilityAdmissionBoundary& admission_boundary);
};

} // namespace bluesky::operations
