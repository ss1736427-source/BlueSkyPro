#pragma once
#include "ai_runtime_continuity.hpp"
#include "ai_trace_recorder.hpp"
#include <cstdint>
#include <string>
#include <vector>
namespace bluesky::ai {
enum class LifecycleEventKind { SystemEvent, TaskCreated, TaskAssigned, AgentResultAccepted, TaskCompleted, TaskConflicted, TaskTimedOut, TaskFailed, ProposalCreated, ProposalSubmitted, RecoveryStarted, RecoveryAccepted, RecoveryRejected };
struct LifecycleEvent { std::string event_id; std::string correlation_id; LifecycleEventKind kind; std::string subject_id; std::uint64_t sequence{0}; };
class AiRuntimeLifecycle final {
public:
 explicit AiRuntimeLifecycle(AiRuntimeContinuity& runtime) noexcept : runtime_(runtime) {}
 bool record(const LifecycleEvent&) noexcept;
 bool validate_sequence(const std::string&) const noexcept;
 std::vector<LifecycleEvent> events_for(const std::string&) const;
private:
 static TraceStage trace_stage(LifecycleEventKind) noexcept;
 AiRuntimeContinuity& runtime_;
 std::vector<LifecycleEvent> events_;
};
} // namespace bluesky::ai
