#include "authorization_readiness_gate.hpp"

#include <cassert>

using namespace bluesky::planning;

static AuthorizationReadinessInput restricted() {
    return {true, true, false, false, false, false};
}

int main() {
    auto missing = AuthorizationReadinessGate::assess(restricted());
    assert(missing.status == AuthorizationReadinessStatus::Block);

    auto invalid = restricted();
    invalid.authorization_present = true;
    invalid.authorization_valid = false;
    assert(AuthorizationReadinessGate::assess(invalid).status ==
           AuthorizationReadinessStatus::Block);

    auto out_of_scope = restricted();
    out_of_scope.authorization_present = true;
    out_of_scope.authorization_valid = true;
    out_of_scope.authorization_scope_matches = false;
    assert(AuthorizationReadinessGate::assess(out_of_scope).status ==
           AuthorizationReadinessStatus::Block);

    auto needs_confirmation = out_of_scope;
    needs_confirmation.authorization_scope_matches = true;
    assert(AuthorizationReadinessGate::assess(needs_confirmation).status ==
           AuthorizationReadinessStatus::Review);

    auto ready = needs_confirmation;
    ready.pilot_confirmation_present = true;
    assert(AuthorizationReadinessGate::assess(ready).status ==
           AuthorizationReadinessStatus::Ready);

    AuthorizationReadinessInput unrestricted{};
    assert(AuthorizationReadinessGate::assess(unrestricted).status ==
           AuthorizationReadinessStatus::NotApplicable);

    return 0;
}
