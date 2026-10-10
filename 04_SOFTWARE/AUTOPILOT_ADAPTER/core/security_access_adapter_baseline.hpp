#pragma once
#include "universal_security_access_adapter.hpp"
namespace bluesky::security {
class SecurityAccessAdapterBaseline final : public UniversalSecurityAccessAdapter { public: SecurityAccessResult authorize(const SecurityAccessRequest& request) const override; };
} // namespace bluesky::security
