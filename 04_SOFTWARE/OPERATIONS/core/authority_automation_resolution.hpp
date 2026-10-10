#pragma once

#include "operational_orchestrator_contract.hpp"

#include <string>

namespace bluesky::operations {

enum class Authority { None, System, Role, ExplicitHumanDecision };
enum class ResolutionStatus { Blocked, Executable, RequiresHuman };

struct AuthorityContext {
    bool system_authorized{false};
    bool role_authorized{false};
    bool human_decision_granted{false};
};

struct ExecutionResolution {
    ResolutionStatus status{ResolutionStatus::Blocked};
    Authority authority{Authority::None};
    AutomationLevel automation{AutomationLevel::HumanGuided};
    std::string reason;
};

inline ExecutionResolution resolve_execution(const Action& action,
                                             const AuthorityContext& context) {
    if (!action.safe) {
        return {ResolutionStatus::Blocked, Authority::None, action.automation,
                "safety_gate"};
    }
    if (!action.regulatory_ok) {
        return {ResolutionStatus::Blocked, Authority::None, action.automation,
                "regulatory_gate"};
    }
    if (!action.energy_ok) {
        return {ResolutionStatus::Blocked, Authority::None, action.automation,
                "energy_gate"};
    }
    if (!action.authorized) {
        return {ResolutionStatus::Blocked, Authority::None, action.automation,
                "action_not_authorized"};
    }

    switch (action.automation) {
        case AutomationLevel::Automatic:
            if (context.system_authorized) {
                return {ResolutionStatus::Executable, Authority::System,
                        action.automation, "system_authorized"};
            }
            return {ResolutionStatus::RequiresHuman, Authority::Role,
                    AutomationLevel::HumanGuided, "system_authority_required"};

        case AutomationLevel::HumanGuided:
            if (context.role_authorized) {
                return {ResolutionStatus::RequiresHuman, Authority::Role,
                        action.automation, "role_action_required"};
            }
            return {ResolutionStatus::Blocked, Authority::None,
                    action.automation, "role_authorization_required"};

        case AutomationLevel::HumanDecision:
            if (context.human_decision_granted) {
                return {ResolutionStatus::Executable, Authority::ExplicitHumanDecision,
                        action.automation, "human_decision_granted"};
            }
            return {ResolutionStatus::RequiresHuman, Authority::ExplicitHumanDecision,
                    action.automation, "human_decision_required"};
    }

    return {ResolutionStatus::Blocked, Authority::None, action.automation,
            "unknown_automation_level"};
}

} // namespace bluesky::operations
