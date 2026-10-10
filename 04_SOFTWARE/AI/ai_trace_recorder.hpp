#pragma once
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
namespace bluesky::ai {
enum class TraceStage { SystemEvent, AiTask, Orchestrator, Agent, Context, Result, Proposal, Validation, Authorization, Execution, ExecutionResult };
struct TraceEvent { std::string event_id; std::string correlation_id; TraceStage stage; std::string subject_id; std::string timestamp; std::string detail; };
class AiTraceRecorder final {
public:
 bool append(const TraceEvent&);
 std::vector<TraceEvent> events_for(const std::string&) const;
 std::size_t size() const noexcept { return events_.size(); }
private:
 std::vector<TraceEvent> events_;
};
}
