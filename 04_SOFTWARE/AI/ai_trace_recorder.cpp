#include "ai_trace_recorder.hpp"
namespace bluesky::ai {
bool AiTraceRecorder::append(const TraceEvent& e) {
 if(e.event_id.empty()||e.correlation_id.empty()||e.subject_id.empty()||e.timestamp.empty()) return false;
 for(const auto& x:events_) if(x.event_id==e.event_id) return false;
 events_.push_back(e); return true;
}
std::vector<TraceEvent> AiTraceRecorder::events_for(const std::string& c) const {
 std::vector<TraceEvent> out; for(const auto& e:events_) if(e.correlation_id==c) out.push_back(e); return out;
}
}
