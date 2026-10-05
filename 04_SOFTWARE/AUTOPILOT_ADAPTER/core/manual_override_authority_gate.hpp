#pragma once

#include <string>

namespace bluesky::authority {

enum class AuthorityState {
    AutonomousMission,
    GuidedManualSupervised,
    Failsafe,
    ReturnRecovery,
    EmergencyAbort,
    CommunicationLost,
    Unknown
};

enum class CommandClass {
    MissionManagement,
    Supervisory,
    RecoveryNonInterfering,
    EmergencyAbort,
    ManualControl,
    Configuration,
    LowLevelActuation
};

enum class RejectionReason {
    None,
    UnknownAuthority,
    StateDoesNotAdmitCommand,
    ExplicitApprovalRequired,
    OnboardFailsafeOwnsAuthority,
    CommunicationUnavailable,
    ForbiddenCommandClass
};

struct AuthorityRequest {
    AuthorityState state{AuthorityState::Unknown};
    CommandClass command{CommandClass::MissionManagement};
    bool explicitlyApproved{false};
};

struct AuthorityDecision {
    bool admitted{false};
    RejectionReason reason{RejectionReason::UnknownAuthority};
    std::string detail;
};

class ManualOverrideAuthorityGate {
public:
    static AuthorityDecision evaluate(const AuthorityRequest& request);
};

} // namespace bluesky::authority
