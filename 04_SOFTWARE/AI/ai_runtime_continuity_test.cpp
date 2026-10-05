#include "ai_runtime_continuity.hpp"
#include <cassert>
using namespace bluesky::ai;
int main() {
    AiRuntimeContinuity runtime;
    assert(runtime.establish_baseline("CFG-001","MODEL-001"));
    assert(runtime.enter_offline());
    AgentRegistration agent;
    agent.identity.agent_id="agent-1"; agent.identity.agent_type="Mission"; agent.identity.authority_class="proposal-only";
    agent.capabilities.insert("route-analysis");
    assert(runtime.orchestrator().register_agent(agent));
    OrchestrationTask task;
    task.task_id="task-1"; task.correlation_id="corr-1"; task.required_capability="route-analysis";
    task.candidate_agent_ids={"agent-1"}; task.deadline_tick=10;
    assert(runtime.create_task(task)); assert(runtime.assign_task("task-1"));
    AgentResult result;
    result.task_id="task-1"; result.agent_id="agent-1"; result.correlation_id="corr-1";
    result.result_id="result-1"; result.result_digest="digest-1";
    assert(runtime.record_result(result));
    Proposal proposal;
    proposal.proposal_id="proposal-1"; proposal.task_id="task-1"; proposal.source_agent=agent.identity;
    proposal.orchestrator_id="orchestrator-1"; proposal.context_reference="ctx-1";
    assert(runtime.submit_proposal(ProposalSubmission{proposal,"result-1","proposal-digest-1"}));
    assert(runtime.append_trace(TraceEvent{"event-1","corr-1",TraceStage::Proposal,"proposal-1","t1","offline proposal"}));
    assert(runtime.continuity().pending_task_count==1);
    assert(runtime.continuity().retained_proposal_count==1);
    assert(runtime.continuity().retained_trace_count==1);
    assert(runtime.continuity().authority_model_preserved);
    assert(!runtime.accept_external_result());
    Proposal executable=proposal;
    executable.state=ProposalState::Authorized;
    executable.validation=AuthorityDecision::Allowed;
    executable.safety=AuthorityDecision::Allowed;
    executable.authorization=AuthorityDecision::Allowed;
    assert(runtime.transition_to_executing(executable));
    Proposal bypass=proposal;
    bypass.state=ProposalState::Authorized; bypass.validation=AuthorityDecision::Allowed; bypass.safety=AuthorityDecision::Allowed;
    assert(!runtime.transition_to_executing(bypass));
    assert(runtime.begin_recovery());
    assert(runtime.complete_recovery(RecoveryDecision::Rejected));
    assert(runtime.continuity().mode==ConnectivityMode::Degraded);
    assert(!runtime.accept_external_result());
    assert(runtime.begin_recovery());
    assert(runtime.complete_recovery(RecoveryDecision::Accepted));
    assert(runtime.continuity().mode==ConnectivityMode::Online);
    assert(runtime.continuity().authority_model_preserved);
    assert(!runtime.accept_external_result());
    return 0;
}
