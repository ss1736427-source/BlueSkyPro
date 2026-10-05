#include "fault_recovery_adapter_baseline.hpp"
#include <cassert>
int main(){ using namespace bluesky::fault; FaultRecoveryAdapterBaseline a; auto e=a.readEvent(); assert(e.sourceId=="SRC-19"); assert(e.valid); auto r=a.acceptEvent(e); assert(r.accepted); e.sourceId="BAD"; r=a.acceptEvent(e); assert(!r.accepted && r.error==FaultRecoveryError::InvalidData); e.sourceId="SRC-19"; e.stale=true; r=a.acceptEvent(e); assert(!r.accepted && r.error==FaultRecoveryError::StaleData); return 0; }
