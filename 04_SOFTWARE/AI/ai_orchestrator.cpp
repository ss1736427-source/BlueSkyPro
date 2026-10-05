#include "ai_orchestrator.hpp"
namespace bluesky::ai {
bool AiOrchestrator::register_agent(const AgentRegistration& a) {
 if (a.identity.agent_id.empty() || a.identity.authority_class.empty() || agents_.contains(a.identity.agent_id)) return false;
 agents_.emplace(a.identity.agent_id,a); return true;
}
bool AiOrchestrator::set_agent_health(const std::string& id, AgentHealth h) {
 auto i=agents_.find(id); if(i==agents_.end()) return false; i->second.health=h; return true;
}
bool AiOrchestrator::create_task(const OrchestrationTask& t) {
 if(t.task_id.empty()||t.correlation_id.empty()||t.required_capability.empty()||tasks_.contains(t.task_id)||t.deadline_tick<current_tick_) return false;
 tasks_.emplace(t.task_id,t); return true;
}
bool AiOrchestrator::assign_task(const std::string& id) {
 auto i=tasks_.find(id); if(i==tasks_.end()) return false; auto& t=i->second;
 if(t.state==TaskState::Completed||t.state==TaskState::Conflicted||t.state==TaskState::TimedOut||t.state==TaskState::Failed) return false;
 t.assigned_agent_ids.clear();
 for(const auto& aid:t.candidate_agent_ids){ auto a=agents_.find(aid); if(a==agents_.end()) continue; if(a->second.health!=AgentHealth::Available) continue; if(!a->second.capabilities.contains(t.required_capability)) continue; t.assigned_agent_ids.insert(aid); }
 if(t.assigned_agent_ids.empty()){t.state=TaskState::Unavailable;return false;} t.state=TaskState::Assigned; return true;
}
bool AiOrchestrator::record_result(const AgentResult& r) {
 auto i=tasks_.find(r.task_id); if(i==tasks_.end()) return false; auto& t=i->second;
 if(r.correlation_id!=t.correlation_id||!t.assigned_agent_ids.contains(r.agent_id)) return false;
 if(t.state==TaskState::TimedOut||t.state==TaskState::Failed||t.state==TaskState::Conflicted) return false;
 for(const auto& x:t.results) if(x.agent_id==r.agent_id) return false;
 t.results.push_back(r); recompute_task_state(t); return true;
}
void AiOrchestrator::recompute_task_state(OrchestrationTask& t) {
 if(t.results.empty()){t.state=TaskState::Running;return;}
 for(const auto& r:t.results) if(r.failed||!r.valid){t.state=TaskState::Failed;return;}
 std::unordered_set<std::string> d; for(const auto& r:t.results)d.insert(r.result_digest);
 if(d.size()>1){t.state=TaskState::Conflicted;return;}
 t.state=(t.results.size()==t.assigned_agent_ids.size())?TaskState::Completed:TaskState::Running;
}
bool AiOrchestrator::advance_time(std::uint64_t tick) {
 if (tick < current_tick_) return false;
 current_tick_ = tick;
 for(auto& [id,t]:tasks_) if(t.state!=TaskState::Completed&&t.state!=TaskState::Conflicted&&t.state!=TaskState::Failed&&t.state!=TaskState::TimedOut&&current_tick_>t.deadline_tick)t.state=TaskState::TimedOut;
 return true;
}
const OrchestrationTask* AiOrchestrator::task(const std::string& id) const noexcept { auto i=tasks_.find(id); return i==tasks_.end()?nullptr:&i->second; }
} // namespace bluesky::ai
