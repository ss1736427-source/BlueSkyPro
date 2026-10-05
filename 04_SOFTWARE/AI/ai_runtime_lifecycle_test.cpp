#include "ai_runtime_lifecycle.hpp"
#include <cassert>
using namespace bluesky::ai;
int main(){
 AiRuntimeContinuity runtime; AiRuntimeLifecycle lifecycle(runtime); const std::string c="CORR-LIFE-001";
 assert(lifecycle.record({"E1",c,LifecycleEventKind::SystemEvent,"SYS-1",1}));
 assert(lifecycle.record({"E2",c,LifecycleEventKind::TaskCreated,"TASK-1",2}));
 assert(lifecycle.record({"E3",c,LifecycleEventKind::TaskAssigned,"TASK-1",3}));
 assert(lifecycle.record({"E4",c,LifecycleEventKind::AgentResultAccepted,"R-1",4}));
 assert(lifecycle.record({"E5",c,LifecycleEventKind::TaskCompleted,"TASK-1",5}));
 assert(lifecycle.validate_sequence(c)); assert(lifecycle.events_for(c).size()==5);
 const auto trace=runtime.trace(c); assert(trace.size()==5);
 assert(trace[0].stage==TraceStage::SystemEvent); assert(trace[1].stage==TraceStage::AiTask); assert(trace[2].stage==TraceStage::Orchestrator); assert(trace[3].stage==TraceStage::Agent); assert(trace[4].stage==TraceStage::Result);
 assert(trace[4].event_id=="E5"&&trace[4].correlation_id==c);
 assert(!lifecycle.record({"E5-DUP",c,LifecycleEventKind::TaskCompleted,"TASK-1",5}));
 const std::string conflict="CORR-LIFE-002"; assert(lifecycle.record({"F1",conflict,LifecycleEventKind::SystemEvent,"SYS-2",1})); assert(lifecycle.record({"F2",conflict,LifecycleEventKind::TaskCreated,"TASK-2",2})); assert(lifecycle.record({"F3",conflict,LifecycleEventKind::TaskConflicted,"TASK-2",3})); assert(lifecycle.validate_sequence(conflict));
 const std::string gap="CORR-LIFE-003"; assert(lifecycle.record({"G1",gap,LifecycleEventKind::SystemEvent,"SYS-3",1})); assert(lifecycle.record({"G3",gap,LifecycleEventKind::TaskTimedOut,"TASK-3",3})); assert(!lifecycle.validate_sequence(gap));
 const std::string open="CORR-LIFE-004"; assert(lifecycle.record({"H1",open,LifecycleEventKind::SystemEvent,"SYS-4",1})); assert(lifecycle.record({"H2",open,LifecycleEventKind::TaskCreated,"TASK-4",2})); assert(!lifecycle.validate_sequence(open)); return 0;
}
