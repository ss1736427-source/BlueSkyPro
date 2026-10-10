#include "ai_orchestrator.hpp"
#include <cassert>
using namespace bluesky::ai;
static AgentRegistration agent(const char* id){ AgentRegistration a; a.identity.agent_id=id; a.identity.authority_class="non-authoritative"; a.capabilities.insert("route-analysis"); return a; }
static OrchestrationTask task(const char* id="TASK-072"){ OrchestrationTask t; t.task_id=id; t.correlation_id="CORR-072"; t.required_capability="route-analysis"; t.candidate_agent_ids={"mission-agent","safety-agent"}; t.deadline_tick=10; return t; }
int main(){
 AiOrchestrator o;
 assert(o.register_agent(agent("mission-agent"))); assert(o.register_agent(agent("safety-agent"))); assert(!o.register_agent(agent("mission-agent")));
 assert(o.create_task(task())); assert(o.assign_task("TASK-072"));
 assert(o.record_result({"TASK-072","mission-agent","CORR-072","R1","same",true,false,{}}));
 assert(o.task("TASK-072")->state==TaskState::Running);
 assert(o.record_result({"TASK-072","safety-agent","CORR-072","R2","same",true,false,{}}));
 assert(o.task("TASK-072")->state==TaskState::Completed);
 assert(!o.record_result({"TASK-072","safety-agent","CORR-072","R2","same",true,false,{}}));
 assert(!o.record_result({"TASK-072","mission-agent","WRONG","R3","same",true,false,{}}));
 auto u=task("TASK-072-UNAVAILABLE"); u.candidate_agent_ids={"offline"}; assert(o.create_task(u)); assert(!o.assign_task(u.task_id)); assert(o.task(u.task_id)->state==TaskState::Unavailable);
 auto c=task("TASK-072-CONFLICT"); assert(o.create_task(c)); assert(o.assign_task(c.task_id));
 assert(o.record_result({"TASK-072-CONFLICT","mission-agent","CORR-072","C1","A",true,false,{}}));
 assert(o.record_result({"TASK-072-CONFLICT","safety-agent","CORR-072","C2","B",true,false,{}}));
 assert(o.task(c.task_id)->state==TaskState::Conflicted);
 auto x=task("TASK-072-TIMEOUT"); x.deadline_tick=5; assert(o.create_task(x)); assert(o.assign_task(x.task_id)); assert(o.advance_time(6)); assert(o.task(x.task_id)->state==TaskState::TimedOut);
 assert(!o.record_result({"TASK-072-TIMEOUT","mission-agent","CORR-072","LATE","same",true,false,{}}));
 return 0;
}