#pragma once
#include "ai_offline_continuity.hpp"
#include "ai_orchestrator.hpp"
#include "ai_proposal_aggregator.hpp"
#include "ai_trace_recorder.hpp"

namespace bluesky::ai {
class AiRuntimeContinuity final {
public:
    bool establish_baseline(const std::string&, const std::string&) noexcept;
    bool enter_offline() noexcept;
    bool enter_degraded() noexcept;
    bool begin_recovery() noexcept;
    bool complete_recovery(RecoveryDecision) noexcept;
    bool create_task(const OrchestrationTask&) noexcept;
    bool assign_task(const std::string&) noexcept;
    bool record_result(const AgentResult&) noexcept;
    bool submit_proposal(const ProposalSubmission&) noexcept;
    bool append_trace(const TraceEvent&) noexcept;
    bool transition_to_executing(Proposal&) const noexcept;
    bool accept_external_result() const noexcept;
    const OfflineContinuitySnapshot& continuity() const noexcept;
    const OrchestrationTask* task(const std::string&) const noexcept;
    const ProposalSubmission* proposal(const std::string&) const noexcept;
    std::vector<TraceEvent> trace(const std::string&) const;
    AiOrchestrator& orchestrator() noexcept { return orchestrator_; }
private:
    bool refresh_retained_state() noexcept;
    OfflineContinuity continuity_;
    AiOrchestrator orchestrator_;
    AiTraceRecorder trace_recorder_;
    ProposalAggregator proposal_aggregator_;
};
} // namespace bluesky::ai
