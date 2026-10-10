#include "authorization_readiness_gate.hpp"

#include <vector>

namespace bluesky::planning {

AuthorizationReadinessResult AuthorizationReadinessGate::assess(
    const AuthorizationReadinessInput& input) noexcept {
    AuthorizationReadinessResult result;

    if (!input.regulatory_restriction_applies || !input.authorization_required) {
        result.status = AuthorizationReadinessStatus::NotApplicable;
        return result;
    }

    result.status = AuthorizationReadinessStatus::Block;
    result.findings.push_back(
        {AuthorizationFindingCode::AuthorizationRequired,
         "operation is within a regulatory restriction requiring authorization"});

    if (!input.authorization_present) {
        result.findings.push_back(
            {AuthorizationFindingCode::AuthorizationMissing,
             "current authorization evidence is missing"});
        return result;
    }

    if (!input.authorization_valid) {
        result.findings.push_back(
            {AuthorizationFindingCode::AuthorizationInvalid,
             "authorization is not current and valid"});
        return result;
    }

    if (!input.authorization_scope_matches) {
        result.findings.push_back(
            {AuthorizationFindingCode::AuthorizationOutOfScope,
             "authorization does not match the operation scope"});
        return result;
    }

    if (!input.pilot_confirmation_present) {
        result.status = AuthorizationReadinessStatus::Review;
        result.findings.push_back(
            {AuthorizationFindingCode::PilotConfirmationRequired,
             "pilot confirmation of the validated authorization is required"});
        return result;
    }

    result.status = AuthorizationReadinessStatus::Ready;
    result.findings.clear();
    return result;
}

}  // namespace bluesky::planning
