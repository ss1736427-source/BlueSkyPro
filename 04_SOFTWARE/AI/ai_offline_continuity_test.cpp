#include "ai_offline_continuity.hpp"
#include <cassert>
using namespace bluesky::ai;
int main(){
 OfflineContinuity c;
 assert(!c.enter_offline());
 assert(c.establish_baseline("CFG-001","MODEL-LOCAL-001"));
 assert(c.enter_offline());
 assert(c.snapshot().mode==ConnectivityMode::Offline);
 assert(!c.accept_external_result());
 assert(c.update_retained_state(2,3,11));
 assert(c.snapshot().pending_task_count==2 && c.snapshot().retained_proposal_count==3 && c.snapshot().retained_trace_count==11);
 assert(c.snapshot().authority_model_preserved && !c.snapshot().external_results_authoritative);
 assert(c.enter_degraded());
 assert(c.snapshot().mode==ConnectivityMode::Degraded);
 assert(c.begin_recovery());
 assert(c.snapshot().mode==ConnectivityMode::Recovering);
 assert(!c.accept_external_result());
 assert(c.complete_recovery(RecoveryDecision::Rejected));
 assert(c.snapshot().mode==ConnectivityMode::Degraded);
 assert(c.begin_recovery());
 assert(c.complete_recovery(RecoveryDecision::Accepted));
 assert(c.snapshot().mode==ConnectivityMode::Online);
 assert(!c.update_retained_state(0,0,0));
 return 0;
}
