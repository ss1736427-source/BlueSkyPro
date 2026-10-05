#pragma once
#include <cstdint>
#include <string>
namespace bluesky::security {
enum class SecurityAccessState { Unknown, Granted, Denied, Expired, Revoked };
enum class SecurityAccessQuality { Unknown, Valid, Stale, Invalid };
enum class SecurityAccessError { None, InvalidIdentity, InvalidCredential, ExpiredCredential, AccessDenied, RevokedCredential, StaleData };
struct SecurityAccessRequest { std::string subjectId; std::string resourceId; std::string operationId; std::int64_t timestampMs{0}; std::string credentialReference; SecurityAccessQuality quality{SecurityAccessQuality::Unknown}; bool valid{false}; };
struct SecurityAccessResult { SecurityAccessState state{SecurityAccessState::Unknown}; SecurityAccessError error{SecurityAccessError::None}; std::int64_t timestampMs{0}; bool authorized{false}; };
class UniversalSecurityAccessAdapter { public: virtual ~UniversalSecurityAccessAdapter() = default; virtual SecurityAccessResult authorize(const SecurityAccessRequest& request) const = 0; };
} // namespace bluesky::security
