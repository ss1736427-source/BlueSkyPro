#include "ai_orchestrator.hpp"
#include "ai_proposal_aggregator.hpp"
#include "ai_trace_recorder.hpp"
#include <cassert>
using namespace bluesky::ai;

static AgentRegistration agent(const char* id) {
    AgentRegistration a;
    a.identity.agent_id = id;
    a.identity.authority_class = "non-authoritative";
    a.capabilities.insert("route-analysis");
    return a;
}

static TraceEvent trace(const char* id, TraceStage stage, const char* subject, const char* detail) {
    return {id, "CORR-INT-001", stage, subject, id, detail};
}

int main() {
    AiOrchestrator orchestrator;
    AiTraceRecorder trace_recorder;
    ProposalAggregator proposals;

    assert(orchestrator.register_agent(agent("mission-agent")));
    assert(orchestrator.register_agent(agent("safety-agent")));

    OrchestrationTask task;
    task.task_id = "TASK-INT-001";
    task.correlation_id = "CORR-INT-001";
    task.required_capability = "route-analysis";
    task.candidate_agent_ids = {"mission-agent", "safety-agent"};
    task.deadline_tick = 10;

    assert(trace_recorder.append(trace("E1", TraceStage::SystemEvent, "SYS-INT-001", "controlled AI event")));
    assert(orchestrator.create_task(task));
    assert(trace_recorder.append(trace("E2", TraceStage::AiTask, "TASK-INT-001", "task created")));
    assert(orchestrator.assign_task("TASK-INT-001"));
    assert(trace_recorder.append(trace("E3", TraceStage::Orchestrator, "ORCH-001", "agents assigned")));
    assert(trace_recorder.append(trace("E4", TraceStage::Agent, "mission-agent", "agent selected")));
    assert(trace_recorder.append(trace("E5", TraceStage::Context, "CTX-INT-001", "bounded context")));
    assert(orchestrator.record_result({"TASK-INT-001", "mission-agent", "CORR-INT-001", "R-1", "DIGEST-A", true, false, {}}));
    assert(orchestrator.record_result({"TASK-INT-001", "safety-agent", "CORR-INT-001", "R-2", "DIGEST-A", true, false, {}}));
    assert(orchestrator.task("TASK-INT-001")->state == TaskState::Completed);
    assert(trace_recorder.append(trace("E6", TraceStage::Result, "R-1", "converged result")));

    Proposal proposal;
    proposal.proposal_id = "P-INT-001";
    proposal.task_id = "TASK-INT-001";
    proposal.source_agent.agent_id = "mission-agent";
    proposal.orchestrator_id = "ORCH-001";
    proposal.state = ProposalState::Created;

    assert(proposals.submit({proposal, "R-1", "PROPOSAL-DIGEST-001"}));
    assert(trace_recorder.append(trace("E7", TraceStage::Proposal, "P-INT-001", "proposal accepted")));
    assert(proposals.get("P-INT-001") != nullptr);

    // The integrated non-authoritative path stops here.
    assert(!ProposalBoundary::can_execute(proposal));
    assert(!ProposalBoundary::transition_to_executing(proposal));

    // Correlation mismatch is rejected by the orchestrator.
    assert(!orchestrator.record_result({"TASK-INT-001", "mission-agent", "WRONG-CORR", "R-3", "DIGEST-A", true, false, {}}));

    // Conflicting agent results are contained as a task conflict.
    OrchestrationTask conflict;
    conflict.task_id = "TASK-INT-002";
    conflict.correlation_id = "CORR-INT-002";
    conflict.required_capability = "route-analysis";
    conflict.candidate_agent_ids = {"mission-agent", "safety-agent"};
    conflict.deadline_tick = 10;
    assert(orchestrator.create_task(conflict));
    assert(orchestrator.assign_task("TASK-INT-002"));
    assert(orchestrator.record_result({"TASK-INT-002", "mission-agent", "CORR-INT-002", "R-4", "DIGEST-X", true, false, {}}));
    assert(orchestrator.record_result({"TASK-INT-002", "safety-agent", "CORR-INT-002", "R-5", "DIGEST-Y", true, false, {}}));
    assert(orchestrator.task("TASK-INT-002")->state == TaskState::Conflicted);

    // Deadline expiry is contained as a task timeout.
    OrchestrationTask timeout;
    timeout.task_id = "TASK-INT-003";
    timeout.correlation_id = "CORR-INT-003";
    timeout.required_capability = "route-analysis";
    timeout.candidate_agent_ids = {"mission-agent"};
    timeout.deadline_tick = 1;
    assert(orchestrator.create_task(timeout));
    assert(orchestrator.assign_task("TASK-INT-003"));
    assert(orchestrator.advance_time(2));
    assert(orchestrator.task("TASK-INT-003")->state == TaskState::TimedOut);

    assert(trace_recorder.events_for("CORR-INT-001").size() == 7);
    return 0;
}
