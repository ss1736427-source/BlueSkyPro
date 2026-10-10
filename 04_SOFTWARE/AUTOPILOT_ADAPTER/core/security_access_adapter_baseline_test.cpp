#include "security_access_adapter_baseline.hpp"
#include <cassert>
using namespace bluesky::security;
int main() {
    SecurityAccessAdapterBaseline adapter; SecurityAccessRequest request;
    request.subjectId="operator-01"; request.resourceId="flight-01"; request.operationId="execute"; request.timestampMs=1000; request.credentialReference="credential-ref"; request.quality=SecurityAccessQuality::Valid; request.valid=true;
    auto granted=adapter.authorize(request); assert(granted.authorized); assert(granted.state==SecurityAccessState::Granted); assert(granted.error==SecurityAccessError::None);
    request.valid=false; auto denied=adapter.authorize(request); assert(!denied.authorized); assert(denied.state==SecurityAccessState::Denied); assert(denied.error==SecurityAccessError::InvalidCredential);
    request.valid=true; request.quality=SecurityAccessQuality::Stale; auto stale=adapter.authorize(request); assert(!stale.authorized); assert(stale.error==SecurityAccessError::StaleData);
    return 0;
}
