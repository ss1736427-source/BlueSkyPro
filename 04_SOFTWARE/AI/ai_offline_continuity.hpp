#pragma once
#include <cstddef>
#include <string>

namespace bluesky::ai {

enum class ConnectivityMode { Online, Offline, Degraded, Recovering };
enum class RecoveryDecision { NotEvaluated, Accepted, Rejected };
struct OfflineContinuitySnapshot {
    ConnectivityMode mode{ConnectivityMode::Online};
    std::string configuration_baseline_id;
    std::string approved_model_id;
    std::size_t pending_task_count{0};
    std::size_t retained_proposal_count{0};
    std::size_t retained_trace_count{0};
    bool authority_model_preserved{true};
    bool external_results_authoritative{false};
};
class OfflineContinuity final {
public:
    bool establish_baseline(const std::string&, const std::string&) noexcept;
    bool enter_offline() noexcept;
    bool enter_degraded() noexcept;
    bool begin_recovery() noexcept;
    bool complete_recovery(RecoveryDecision) noexcept;
    bool update_retained_state(std::size_t, std::size_t, std::size_t) noexcept;
    bool accept_external_result() const noexcept { return false; }
    const OfflineContinuitySnapshot& snapshot() const noexcept { return snapshot_; }
private:
    OfflineContinuitySnapshot snapshot_;
};
}
