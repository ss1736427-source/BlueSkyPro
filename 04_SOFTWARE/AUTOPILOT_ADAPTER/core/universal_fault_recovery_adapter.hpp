#pragma once
#include <cstdint>
#include <string>
namespace bluesky::fault {
enum class FaultRecoveryState { Unknown, Normal, Detected, Recovering, Recovered, Escalated, Unrecoverable };
enum class FaultType { None, CommunicationLoss, NavigationLoss, SensorFault, AutopilotFault, PowerFault, MissionFault, ExternalFault };
enum class RecoveryAction { None, Hold, Retry, Reconfigure, ReturnToLaunch, ReturnToOperator, Land, AbortMission, EmergencyStop };
enum class FaultQuality { Unknown, Valid, Stale, Invalid, Degraded };
enum class FaultRecoveryError { None, InvalidData, StaleData, Unsupported, RecoveryRejected, InternalAdapterError };
struct FaultEvent { std::string sourceId; std::int64_t timestampUtcMs{0}; FaultType type{FaultType::None}; FaultRecoveryState state{FaultRecoveryState::Unknown}; RecoveryAction requestedAction{RecoveryAction::None}; RecoveryAction appliedAction{RecoveryAction::None}; FaultQuality quality{FaultQuality::Unknown}; bool valid{false}; bool stale{false}; };
struct FaultRecoveryResult { bool accepted{false}; FaultRecoveryError error{FaultRecoveryError::None}; std::string reason; };
class UniversalFaultRecoveryAdapter { public: virtual ~UniversalFaultRecoveryAdapter() = default; virtual std::string getSourceId() const = 0; virtual FaultRecoveryState getState() const = 0; virtual FaultEvent readEvent() const = 0; virtual FaultRecoveryResult acceptEvent(const FaultEvent& event) = 0; };
} // namespace bluesky::fault
