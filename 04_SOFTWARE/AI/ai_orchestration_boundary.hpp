#pragma once

#include <string>
#include <vector>

namespace bluesky::ai {

enum class ProposalState {
    Created,
    Validating,
    SafetyReview,
    WaitingAuthorization,
    Authorized,
    Executing,
    Executed,
    Rejected,
    Blocked,
    Expired,
    Conflicted,
    Cancelled,
    Failed
};

enum class AuthorityDecision {
    NotEvaluated,
    Allowed,
    Denied
};

struct MessageEnvelope {
    std::string message_id;
    std::string timestamp;
    std::string source;
    std::string destination;
    std::string message_type;
    int priority{4};
    std::string correlation_id;
    std::string schema_version;
};

struct AgentIdentity {
    std::string agent_id;
    std::string agent_type;
    std::string model_id;
    std::string model_version;
    std::string configuration_id;
    std::string authority_class;
};

struct Proposal {
    std::string proposal_id;
    std::string task_id;
    AgentIdentity source_agent;
    std::string orchestrator_id;
    std::string context_reference;
    std::string affected_mission_id;
    std::string affected_mission_revision;
    std::vector<std::string> proposed_changes;
    ProposalState state{ProposalState::Created};
    AuthorityDecision validation{AuthorityDecision::NotEvaluated};
    AuthorityDecision safety{AuthorityDecision::NotEvaluated};
    AuthorityDecision authorization{AuthorityDecision::NotEvaluated};
};

class ProposalBoundary final {
public:
    static bool can_execute(const Proposal& proposal) noexcept {
        return proposal.state == ProposalState::Authorized &&
               proposal.validation == AuthorityDecision::Allowed &&
               proposal.safety == AuthorityDecision::Allowed &&
               proposal.authorization == AuthorityDecision::Allowed;
    }

    static bool transition_to_executing(Proposal& proposal) noexcept {
        if (!can_execute(proposal)) {
            return false;
        }
        proposal.state = ProposalState::Executing;
        return true;
    }
};

} // namespace bluesky::ai
