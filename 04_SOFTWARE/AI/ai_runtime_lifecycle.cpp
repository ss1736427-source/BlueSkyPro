#include "ai_runtime_lifecycle.hpp"
#include <algorithm>
namespace bluesky::ai {
TraceStage AiRuntimeLifecycle::trace_stage(LifecycleEventKind kind) noexcept {
 switch(kind){
 case LifecycleEventKind::SystemEvent: case LifecycleEventKind::RecoveryStarted: case LifecycleEventKind::RecoveryAccepted: case LifecycleEventKind::RecoveryRejected: return TraceStage::SystemEvent;
 case LifecycleEventKind::TaskCreated:return TraceStage::AiTask;
 case LifecycleEventKind::TaskAssigned:return TraceStage::Orchestrator;
 case LifecycleEventKind::AgentResultAccepted:return TraceStage::Agent;
 case LifecycleEventKind::ProposalCreated: case LifecycleEventKind::ProposalSubmitted:return TraceStage::Proposal;
 case LifecycleEventKind::TaskCompleted: case LifecycleEventKind::TaskConflicted: case LifecycleEventKind::TaskTimedOut: case LifecycleEventKind::TaskFailed:return TraceStage::Result;
 }
 return TraceStage::SystemEvent;
}
bool AiRuntimeLifecycle::record(const LifecycleEvent& e) noexcept {
 if(e.event_id.empty()||e.correlation_id.empty()||e.subject_id.empty()||e.sequence==0)return false;
 for(const auto& x:events_) { if(x.event_id==e.event_id)return false; if(x.correlation_id==e.correlation_id&&x.sequence==e.sequence)return false; }
 TraceEvent t{e.event_id,e.correlation_id,trace_stage(e.kind),e.subject_id,"seq:"+std::to_string(e.sequence),"lifecycle-sequence:"+std::to_string(e.sequence)};
 if(!runtime_.append_trace(t))return false;
 events_.push_back(e); return true;
}
std::vector<LifecycleEvent> AiRuntimeLifecycle::events_for(const std::string& c) const {
 std::vector<LifecycleEvent> r; for(const auto& e:events_)if(e.correlation_id==c)r.push_back(e);
 std::sort(r.begin(),r.end(),[](const auto&a,const auto&b){return a.sequence<b.sequence;}); return r;
}
bool AiRuntimeLifecycle::validate_sequence(const std::string& c) const noexcept {
 const auto e=events_for(c); if(e.empty())return false;
 for(std::size_t i=1;i<e.size();++i)if(e[i].sequence!=e[i-1].sequence+1)return false;
 const auto k=e.back().kind;
 return k==LifecycleEventKind::TaskCompleted||k==LifecycleEventKind::TaskConflicted||k==LifecycleEventKind::TaskTimedOut||k==LifecycleEventKind::TaskFailed||k==LifecycleEventKind::RecoveryAccepted||k==LifecycleEventKind::RecoveryRejected;
}
} // namespace bluesky::ai
