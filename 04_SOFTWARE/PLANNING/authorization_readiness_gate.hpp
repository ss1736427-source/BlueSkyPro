#pragma once

#include <string>
#include <vector>

namespace bluesky::planning {

enum class AuthorizationReadinessStatus {
    Ready,
    NotApplicable,
    Review,
    Block
};

enum class AuthorizationFindingCode {
    AuthorizationRequired,
    AuthorizationMissing,
    AuthorizationInvalid,
    AuthorizationOutOfScope,
    PilotConfirmationRequired
};

struct AuthorizationReadinessInput {
    bool regulatory_restriction_applies{false};
    bool authorization_required{false};
    bool authorization_present{false};
    bool authorization_valid{false};
    bool authorization_scope_matches{false};
    bool pilot_confirmation_present{false};
};

struct AuthorizationReadinessFinding {
    AuthorizationFindingCode code;
    std::string detail;
};

struct AuthorizationReadinessResult {
    AuthorizationReadinessStatus status{AuthorizationReadinessStatus::NotApplicable};
    std::vector<AuthorizationReadinessFinding> findings;
};

class AuthorizationReadinessGate {
public:
    static AuthorizationReadinessResult assess(
        const AuthorizationReadinessInput& input) noexcept;
};

}  // namespace bluesky::planning
