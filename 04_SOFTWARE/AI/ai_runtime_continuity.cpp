#include "ai_runtime_continuity.hpp"
namespace bluesky::ai {
bool AiRuntimeContinuity::establish_baseline(const std::string& c,const std::string& m) noexcept { return continuity_.establish_baseline(c,m); }
bool AiRuntimeContinuity::enter_offline() noexcept { return continuity_.enter_offline(); }
bool AiRuntimeContinuity::enter_degraded() noexcept { return continuity_.enter_degraded(); }
bool AiRuntimeContinuity::begin_recovery() noexcept { return continuity_.begin_recovery(); }
bool AiRuntimeContinuity::complete_recovery(RecoveryDecision d) noexcept { return continuity_.complete_recovery(d); }
bool AiRuntimeContinuity::create_task(const OrchestrationTask& t) noexcept { if(!orchestrator_.create_task(t)) return false; return refresh_retained_state(); }
bool AiRuntimeContinuity::assign_task(const std::string& id) noexcept { if(!orchestrator_.assign_task(id)) return false; return refresh_retained_state(); }
bool AiRuntimeContinuity::record_result(const AgentResult& r) noexcept { if(!orchestrator_.record_result(r)) return false; return refresh_retained_state(); }
bool AiRuntimeContinuity::submit_proposal(const ProposalSubmission& s) noexcept { if(!proposal_aggregator_.submit(s)) return false; return refresh_retained_state(); }
bool AiRuntimeContinuity::append_trace(const TraceEvent& e) noexcept { if(!trace_recorder_.append(e)) return false; return refresh_retained_state(); }
bool AiRuntimeContinuity::transition_to_executing(Proposal& p) const noexcept { return ProposalBoundary::transition_to_executing(p); }
bool AiRuntimeContinuity::accept_external_result() const noexcept { return continuity_.accept_external_result(); }
const OfflineContinuitySnapshot& AiRuntimeContinuity::continuity() const noexcept { return continuity_.snapshot(); }
const OrchestrationTask* AiRuntimeContinuity::task(const std::string& id) const noexcept { return orchestrator_.task(id); }
const ProposalSubmission* AiRuntimeContinuity::proposal(const std::string& id) const noexcept { return proposal_aggregator_.get(id); }
std::vector<TraceEvent> AiRuntimeContinuity::trace(const std::string& c) const { return trace_recorder_.events_for(c); }
bool AiRuntimeContinuity::refresh_retained_state() noexcept {
    const auto t=orchestrator_.task_count(), p=proposal_aggregator_.size(), r=trace_recorder_.size();
    return continuity_.update_retained_state(t,p,r) || continuity_.snapshot().mode==ConnectivityMode::Online;
}
} // namespace bluesky::ai
