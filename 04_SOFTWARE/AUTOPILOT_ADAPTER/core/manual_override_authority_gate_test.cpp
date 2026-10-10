#include "manual_override_authority_gate.hpp"

#include <cassert>

using namespace bluesky::authority;

int main() {
    auto decision = ManualOverrideAuthorityGate::evaluate(
        {AuthorityState::AutonomousMission, CommandClass::MissionManagement, false});
    assert(decision.admitted);

    decision = ManualOverrideAuthorityGate::evaluate(
        {AuthorityState::AutonomousMission, CommandClass::ManualControl, true});
    assert(!decision.admitted);
    assert(decision.reason == RejectionReason::StateDoesNotAdmitCommand);

    decision = ManualOverrideAuthorityGate::evaluate(
        {AuthorityState::GuidedManualSupervised, CommandClass::Supervisory, false});
    assert(!decision.admitted);
    assert(decision.reason == RejectionReason::ExplicitApprovalRequired);

    decision = ManualOverrideAuthorityGate::evaluate(
        {AuthorityState::GuidedManualSupervised, CommandClass::Supervisory, true});
    assert(decision.admitted);

    decision = ManualOverrideAuthorityGate::evaluate(
        {AuthorityState::Failsafe, CommandClass::EmergencyAbort, true});
    assert(!decision.admitted);
    assert(decision.reason == RejectionReason::OnboardFailsafeOwnsAuthority);

    decision = ManualOverrideAuthorityGate::evaluate(
        {AuthorityState::ReturnRecovery, CommandClass::RecoveryNonInterfering, false});
    assert(!decision.admitted);
    assert(decision.reason == RejectionReason::ExplicitApprovalRequired);

    decision = ManualOverrideAuthorityGate::evaluate(
        {AuthorityState::ReturnRecovery, CommandClass::RecoveryNonInterfering, true});
    assert(decision.admitted);

    decision = ManualOverrideAuthorityGate::evaluate(
        {AuthorityState::EmergencyAbort, CommandClass::EmergencyAbort, true});
    assert(decision.admitted);

    decision = ManualOverrideAuthorityGate::evaluate(
        {AuthorityState::EmergencyAbort, CommandClass::MissionManagement, true});
    assert(!decision.admitted);

    decision = ManualOverrideAuthorityGate::evaluate(
        {AuthorityState::CommunicationLost, CommandClass::MissionManagement, true});
    assert(!decision.admitted);
    assert(decision.reason == RejectionReason::CommunicationUnavailable);

    decision = ManualOverrideAuthorityGate::evaluate(
        {AuthorityState::Unknown, CommandClass::MissionManagement, true});
    assert(!decision.admitted);
    assert(decision.reason == RejectionReason::UnknownAuthority);

    decision = ManualOverrideAuthorityGate::evaluate(
        {AuthorityState::AutonomousMission, CommandClass::LowLevelActuation, true});
    assert(!decision.admitted);
    return 0;
}
