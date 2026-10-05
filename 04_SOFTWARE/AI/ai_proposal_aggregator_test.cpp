#include "ai_proposal_aggregator.hpp"
#include <cassert>
using namespace bluesky::ai;
int main(){
 ProposalAggregator a; Proposal p; p.proposal_id="P-1"; p.task_id="TASK-072"; p.state=ProposalState::Created;
 p.source_agent.agent_id="mission-agent"; p.orchestrator_id="ORCH-1";
 assert(a.submit({p,"R1","D1"})); assert(a.get("P-1")!=nullptr);
 assert(!a.submit({p,"R2","D2"}));
 p.proposal_id="P-2"; assert(!a.submit({p,"","D2"}));
 p.proposal_id="P-3"; p.state=ProposalState::Authorized; assert(!a.submit({p,"R3","D3"}));
 return 0;
}