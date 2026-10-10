#include "ai_proposal_aggregator.hpp"
#include <unordered_map>
namespace bluesky::ai {
bool ProposalAggregator::submit(const ProposalSubmission& s) {
 if(s.proposal.proposal_id.empty()||s.proposal.task_id.empty()||s.source_result_id.empty()||s.proposal_digest.empty()) return false;
 if(s.proposal.state!=ProposalState::Created) return false;
 if(proposals_.contains(s.proposal.proposal_id)) return false;
 proposals_.emplace(s.proposal.proposal_id,s); return true;
}
const ProposalSubmission* ProposalAggregator::get(const std::string& id) const noexcept { auto i=proposals_.find(id); return i==proposals_.end()?nullptr:&i->second; }
}
