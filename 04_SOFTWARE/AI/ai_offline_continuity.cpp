#include "ai_offline_continuity.hpp"
namespace bluesky::ai {
bool OfflineContinuity::establish_baseline(const std::string& c,const std::string& m) noexcept { if(c.empty()||m.empty()) return false; snapshot_.configuration_baseline_id=c; snapshot_.approved_model_id=m; return true; }
bool OfflineContinuity::enter_offline() noexcept { if(snapshot_.configuration_baseline_id.empty()||snapshot_.approved_model_id.empty()) return false; snapshot_.mode=ConnectivityMode::Offline; snapshot_.external_results_authoritative=false; return true; }
bool OfflineContinuity::enter_degraded() noexcept { if(snapshot_.mode!=ConnectivityMode::Offline&&snapshot_.mode!=ConnectivityMode::Online) return false; snapshot_.mode=ConnectivityMode::Degraded; snapshot_.external_results_authoritative=false; return true; }
bool OfflineContinuity::begin_recovery() noexcept { if(snapshot_.mode!=ConnectivityMode::Offline&&snapshot_.mode!=ConnectivityMode::Degraded) return false; snapshot_.mode=ConnectivityMode::Recovering; snapshot_.external_results_authoritative=false; return true; }
bool OfflineContinuity::complete_recovery(RecoveryDecision d) noexcept { if(snapshot_.mode!=ConnectivityMode::Recovering) return false; if(d==RecoveryDecision::Accepted){snapshot_.mode=ConnectivityMode::Online; snapshot_.external_results_authoritative=false; return true;} if(d==RecoveryDecision::Rejected){snapshot_.mode=ConnectivityMode::Degraded; snapshot_.external_results_authoritative=false; return true;} return false; }
bool OfflineContinuity::update_retained_state(std::size_t t,std::size_t p,std::size_t r) noexcept { if(snapshot_.mode==ConnectivityMode::Online) return false; snapshot_.pending_task_count=t; snapshot_.retained_proposal_count=p; snapshot_.retained_trace_count=r; return true; }
}
