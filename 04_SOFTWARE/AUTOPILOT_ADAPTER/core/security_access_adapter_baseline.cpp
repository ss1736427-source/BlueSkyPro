#include "security_access_adapter_baseline.hpp"
namespace bluesky::security {
SecurityAccessResult SecurityAccessAdapterBaseline::authorize(const SecurityAccessRequest& request) const {
    SecurityAccessResult result; result.timestampMs = request.timestampMs;
    if (request.subjectId.empty() || request.resourceId.empty() || request.operationId.empty()) { result.state=SecurityAccessState::Denied; result.error=SecurityAccessError::InvalidIdentity; return result; }
    if (request.credentialReference.empty()) { result.state=SecurityAccessState::Denied; result.error=SecurityAccessError::InvalidCredential; return result; }
    if (!request.valid || request.quality==SecurityAccessQuality::Invalid) { result.state=SecurityAccessState::Denied; result.error=SecurityAccessError::InvalidCredential; return result; }
    if (request.quality==SecurityAccessQuality::Stale) { result.state=SecurityAccessState::Denied; result.error=SecurityAccessError::StaleData; return result; }
    result.state=SecurityAccessState::Granted; result.error=SecurityAccessError::None; result.authorized=true; return result;
}
} // namespace bluesky::security
