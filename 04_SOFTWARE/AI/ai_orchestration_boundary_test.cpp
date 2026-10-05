#include "ai_orchestration_boundary.hpp"

#include <cassert>

using bluesky::ai::AuthorityDecision;
using bluesky::ai::Proposal;
using bluesky::ai::ProposalBoundary;
using bluesky::ai::ProposalState;

int main() {
    Proposal proposal;
    proposal.proposal_id = "proposal-001";
    proposal.task_id = "task-001";
    proposal.state = ProposalState::Created;

    // Negative path: AI proposal cannot execute directly.
    assert(!ProposalBoundary::can_execute(proposal));
    assert(!ProposalBoundary::transition_to_executing(proposal));
    assert(proposal.state == ProposalState::Created);

    // Validation alone is insufficient.
    proposal.validation = AuthorityDecision::Allowed;
    assert(!ProposalBoundary::can_execute(proposal));

    // Safety approval without authorization is still insufficient.
    proposal.safety = AuthorityDecision::Allowed;
    assert(!ProposalBoundary::can_execute(proposal));

    // Only the complete controlled chain permits execution.
    proposal.authorization = AuthorityDecision::Allowed;
    proposal.state = ProposalState::Authorized;
    assert(ProposalBoundary::can_execute(proposal));
    assert(ProposalBoundary::transition_to_executing(proposal));
    assert(proposal.state == ProposalState::Executing);

    // Rejected/blocked proposals cannot be revived by partial authority flags.
    proposal.state = ProposalState::Rejected;
    assert(!ProposalBoundary::can_execute(proposal));

    return 0;
}
