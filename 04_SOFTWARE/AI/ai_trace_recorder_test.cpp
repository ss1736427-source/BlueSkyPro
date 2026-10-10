#include "ai_trace_recorder.hpp"
#include <cassert>
using namespace bluesky::ai;
int main(){
 AiTraceRecorder r;
 const char* c="CORR-110";
 assert(r.append({"E1",c,TraceStage::SystemEvent,"SYS","T1","event"}));
 assert(r.append({"E2",c,TraceStage::AiTask,"TASK-072","T2","task"}));
 assert(r.append({"E3",c,TraceStage::Orchestrator,"ORCH","T3","assigned"}));
 assert(r.append({"E4",c,TraceStage::Agent,"mission-agent","T4","result"}));
 assert(r.append({"E5",c,TraceStage::Context,"CTX","T5","context"}));
 assert(r.append({"E6",c,TraceStage::Result,"R1","T6","result"}));
 assert(r.append({"E7",c,TraceStage::Proposal,"P1","T7","proposal"}));
 assert(r.append({"E8",c,TraceStage::Validation,"P1","T8","validated"}));
 assert(r.append({"E9",c,TraceStage::Authorization,"P1","T9","authorized"}));
 assert(r.append({"E10",c,TraceStage::Execution,"P1","T10","executed"}));
 assert(r.append({"E11",c,TraceStage::ExecutionResult,"P1","T11","complete"}));
 assert(r.events_for(c).size()==11);
 assert(!r.append({"E11",c,TraceStage::Result,"R2","T12","duplicate"}));
 assert(r.events_for("OTHER").empty());
 return 0;
}