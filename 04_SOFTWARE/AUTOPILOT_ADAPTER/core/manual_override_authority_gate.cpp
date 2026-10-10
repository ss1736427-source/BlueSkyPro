#include "manual_override_authority_gate.hpp"

namespace bluesky::authority {
namespace {

AuthorityDecision reject(RejectionReason reason, const char* detail) {
    return {false, reason, detail};
}

AuthorityDecision admit() {
    return {true, RejectionReason::None, "command admitted by authority policy"};
}

} // namespace

AuthorityDecision ManualOverrideAuthorityGate::evaluate(const AuthorityRequest& request) {
    switch (request.state) {
    case AuthorityState::AutonomousMission:
        if (request.command == CommandClass::MissionManagement)
            return admit();
        return reject(RejectionReason::StateDoesNotAdmitCommand,
                      "autonomous mission state admits mission-management commands only");

    case AuthorityState::GuidedManualSupervised:
        if (request.command != CommandClass::Supervisory)
            return reject(RejectionReason::StateDoesNotAdmitCommand,
                          "guided/manual state admits supervisory commands only");
        if (!request.explicitlyApproved)
            return reject(RejectionReason::ExplicitApprovalRequired,
                          "supervisory command requires explicit policy approval");
        return admit();

    case AuthorityState::Failsafe:
        return reject(RejectionReason::OnboardFailsafeOwnsAuthority,
                      "onboard failsafe retains authority; external command rejected");

    case AuthorityState::ReturnRecovery:
        if (request.command != CommandClass::RecoveryNonInterfering)
            return reject(RejectionReason::StateDoesNotAdmitCommand,
                          "return/recovery admits non-interfering recovery commands only");
        if (!request.explicitlyApproved)
            return reject(RejectionReason::ExplicitApprovalRequired,
                          "recovery command requires explicit policy approval");
        return admit();

    case AuthorityState::EmergencyAbort:
        if (request.command != CommandClass::EmergencyAbort)
            return reject(RejectionReason::StateDoesNotAdmitCommand,
                          "emergency state admits the designated emergency-abort command only");
        if (!request.explicitlyApproved)
            return reject(RejectionReason::ExplicitApprovalRequired,
                          "emergency-abort command requires explicit policy approval");
        return admit();

    case AuthorityState::CommunicationLost:
        return reject(RejectionReason::CommunicationUnavailable,
                      "communication lost; no command may be dispatched");

    case AuthorityState::Unknown:
    default:
        return reject(RejectionReason::UnknownAuthority,
                      "unknown authority state cannot authorize a command");
    }
}

} // namespace bluesky::authority
