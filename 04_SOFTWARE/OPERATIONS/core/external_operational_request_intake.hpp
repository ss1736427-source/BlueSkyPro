#pragma once

#include "operational_action_execution_router.hpp"

#include <string>
#include <vector>

namespace bluesky::operations {

struct ExternalOperationalRequest {
    std::string request_id;
    ActionExecutionBinding binding;
};

enum class ExternalOperationalRequestState { Rejected, Accepted };

struct ExternalOperationalRequestResult {
    ExternalOperationalRequestState state{ExternalOperationalRequestState::Rejected};
    std::string request_id;
    std::string action_id;
    ActionExecutionBinding binding{};
    std::string reason;
    OrchestratorPipelineResult pipeline{};
};

class ExternalOperationalRequestIntake final {
public:
    static ExternalOperationalRequestResult evaluate(
        const ExternalOperationalRequest& request,
        const OperationalSnapshot& snapshot,
        const OrchestratorPipelineContext& context);
};

} // namespace bluesky::operations
