#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace bluesky::safety {

enum class SafetyState {
    Unknown,
    Normal,
    Warning,
    Failsafe,
    Emergency,
    Fault
};

enum class SafetyTrigger {
    None,
    C2Loss,
    NavigationLoss,
    EnergyCritical,
    AutopilotFault,
    GeofenceViolation,
    MissionFailure,
    ExternalEmergency
};

enum class SafetyAction {
    None,
    Hold,
    ReturnToLaunch,
    ReturnToOperator,
    Land,
    AbortMission,
    EmergencyStop
};

enum class SafetyQuality {
    Unknown,
    Invalid,
    Valid,
    Stale,
    Degraded
};

enum class SafetyError {
    None,
    InvalidData,
    StaleData,
    ConfigurationMismatch,
    Unsupported,
    SafetyRejected,
    InternalAdapterError
};

struct SafetyEvent {
    std::string sourceId;
    std::int64_t timestampUtcMs{0};
    SafetyTrigger trigger{SafetyTrigger::None};
    SafetyAction requestedAction{SafetyAction::None};
    SafetyAction appliedAction{SafetyAction::None};
    SafetyState state{SafetyState::Unknown};
    SafetyQuality quality{SafetyQuality::Unknown};
    bool valid{false};
    bool stale{false};
};

struct SafetyResult {
    bool accepted{false};
    SafetyError error{SafetyError::None};
    std::string reason;
};

class UniversalSafetyFailsafeAdapter {
public:
    virtual ~UniversalSafetyFailsafeAdapter() = default;

    virtual std::string getSourceId() const = 0;
    virtual std::string getConfigurationVersion() const = 0;
    virtual SafetyState getState() const = 0;
    virtual SafetyQuality getQuality() const = 0;
    virtual SafetyEvent readEvent() const = 0;
    virtual SafetyResult acceptEvent(const SafetyEvent& event) = 0;
};

} // namespace bluesky::safety
