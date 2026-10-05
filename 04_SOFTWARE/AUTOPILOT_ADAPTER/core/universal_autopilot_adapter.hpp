#pragma once

#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace bluesky::planning {
struct Mission;
struct VehicleEquipmentCapabilityProfile;
}

namespace bluesky::autopilot {

enum class ConnectionState { Disconnected, Connecting, Connected, Degraded, Recovering, Failed };
enum class ExecutionState { Requested, Validating, Rejected, Dispatched, Acknowledged, Executing, Completed, Failed, Cancelled, Timeout, Unknown };
enum class ErrorCode { None, NotConnected, AuthenticationFailed, Unsupported, IncompatibleVersion, CapabilityMissing, InvalidState, SafetyRejected, CommandRejected, Timeout, LinkLost, ProtocolError, MissionMismatch, ConfigurationMismatch, InternalAdapterError };

enum class CompatibilityState { Compatible, Rejected };

struct Identity { std::string vehicleId; std::string vehicleType; std::string autopilot; std::string firmwareVersion; std::string protocol; std::string protocolVersion; };
struct Capabilities { std::vector<std::string> commands; std::vector<std::string> missionFeatures; std::vector<std::string> equipmentFeatures; };
struct CapabilityManifest {
    std::string adapterId;
    std::string vendor;
    std::string protocol;
    std::string protocolVersion;
    std::string fcsName;
    std::string fcsVersion;
    std::vector<std::string> vehicleClasses;
    std::vector<std::string> supportedCommands;
    std::vector<std::string> supportedMissionFeatures;
    std::vector<std::string> supportedParameterFeatures;
    std::vector<std::string> telemetryFeatures;
    std::vector<std::string> equipmentFeatures;
    std::vector<std::string> failsafeFeatures;
    std::vector<std::string> logFeatures;
    std::vector<std::string> securityFeatures;
    std::vector<std::string> knownLimitations;
    std::string verificationStatus;
};
struct CompatibilityRequest {
    std::string requiredProtocol;
    std::string requiredProtocolVersion;
    std::string requiredMissionFeature;
};
struct CompatibilityResult {
    CompatibilityState state{CompatibilityState::Rejected};
    bool compatible{false};
    ErrorCode error{ErrorCode::None};
    std::string reason;
};
struct LinkMetrics { double latencyMs{0.0}; double packetLoss{0.0}; bool healthy{false}; };
struct NormalizedState { std::string value; };
struct CommandResult { std::string commandId; std::string vehicleId; ExecutionState executionState{ExecutionState::Unknown}; bool acknowledged{false}; ErrorCode error{ErrorCode::None}; std::string reason; std::int64_t sourceTimestampMs{0}; std::int64_t adapterTimestampMs{0}; };
struct MissionComparison { bool equal{false}; std::vector<std::string> mismatches; };
struct LogAcquisitionResult { std::string logId; std::string vehicleId; bool accepted{false}; ExecutionState executionState{ExecutionState::Unknown}; ErrorCode error{ErrorCode::None}; std::string reason; std::string sourceReference; std::int64_t sourceTimestampMs{0}; std::int64_t adapterTimestampMs{0}; };

class UniversalAutopilotAdapter {
public:
    virtual ~UniversalAutopilotAdapter() = default;
    virtual Identity getVehicleIdentity() const = 0;
    virtual std::string getAutopilotIdentity() const = 0;
    virtual std::string getFirmwareVersion() const = 0;
    virtual std::string getProtocolVersion() const = 0;
    virtual Capabilities getCapabilities() const = 0;
    virtual std::vector<std::string> getSupportedCommands() const = 0;
    virtual std::vector<std::string> getSupportedMissionFeatures() const = 0;
    virtual std::vector<std::string> getSupportedEquipmentFeatures() const = 0;
    virtual CapabilityManifest getCapabilityManifest() const {
        const auto identity = getVehicleIdentity();
        return {
            identity.autopilot + ":" + identity.protocol,
            "BASELINE",
            identity.protocol,
            identity.protocolVersion,
            identity.autopilot,
            identity.firmwareVersion,
            {identity.vehicleType},
            getSupportedCommands(),
            getSupportedMissionFeatures(),
            {"configuration_read", "configuration_write", "configuration_readback", "configuration_verify"},
            {"navigation", "flight_mode", "health", "failsafe", "energy", "mission", "equipment", "link_metrics"},
            getSupportedEquipmentFeatures(),
            {"failsafe_state"},
            {"flight_log_acquisition"},
            {},
            {"SIL_FIXTURE_ONLY", "NO_REAL_TRANSPORT"},
            "SIL_FIXTURE_ONLY"
        };
    }
    virtual CompatibilityResult checkCompatibility(const CompatibilityRequest& request) const {
        const auto manifest = getCapabilityManifest();
        if (!request.requiredProtocol.empty() && manifest.protocol != request.requiredProtocol) {
            return {CompatibilityState::Rejected, false, ErrorCode::IncompatibleVersion, "PROTOCOL_MISMATCH"};
        }
        if (!request.requiredProtocolVersion.empty() && manifest.protocolVersion != request.requiredProtocolVersion) {
            return {CompatibilityState::Rejected, false, ErrorCode::IncompatibleVersion, "PROTOCOL_VERSION_MISMATCH"};
        }
        if (!request.requiredMissionFeature.empty() && std::find(manifest.supportedMissionFeatures.begin(), manifest.supportedMissionFeatures.end(), request.requiredMissionFeature) == manifest.supportedMissionFeatures.end()) {
            return {CompatibilityState::Rejected, false, ErrorCode::CapabilityMissing, "MISSION_CAPABILITY_MISSING"};
        }
        return {CompatibilityState::Compatible, true, ErrorCode::None, "COMPATIBLE"};
    }
    virtual bool connect() = 0;
    virtual void disconnect() = 0;
    virtual ConnectionState getConnectionState() const = 0;
    virtual LinkMetrics getLinkMetrics() const = 0;
    virtual NormalizedState getNavigationState() const = 0;
    virtual NormalizedState getFlightMode() const = 0;
    virtual NormalizedState getHealthState() const = 0;
    virtual NormalizedState getFailsafeState() const = 0;
    virtual NormalizedState getEnergyState() const = 0;
    virtual NormalizedState getMissionState() const = 0;
    virtual NormalizedState getEquipmentState() const = 0;
    virtual CommandResult arm() = 0;
    virtual CommandResult disarm() = 0;
    virtual CommandResult takeoff(double altitudeM) = 0;
    virtual CommandResult land() = 0;
    virtual CommandResult hold() = 0;
    virtual CommandResult resume() = 0;
    virtual CommandResult rtl() = 0;
    virtual CommandResult gotoPosition(double latDeg, double lonDeg, double altitudeM) = 0;
    virtual CommandResult startMission() = 0;
    virtual CommandResult pauseMission() = 0;
    virtual CommandResult resumeMission() = 0;
    virtual CommandResult abortMission() = 0;
    virtual std::optional<std::string> compileMission(const bluesky::planning::Mission&, const bluesky::planning::VehicleEquipmentCapabilityProfile&) = 0;
    virtual bool uploadMission(const std::string& compiledMission) = 0;
    virtual std::optional<std::string> downloadMission() = 0;
    virtual std::optional<std::string> readBackMission() = 0;
    virtual MissionComparison compareMission(const std::string&, const std::string&) const = 0;
    virtual std::optional<std::string> readBaseline() = 0;
    virtual MissionComparison compare(const std::string&, const std::string&) const = 0;
    virtual CommandResult validate(const std::string&) const = 0;
    virtual CommandResult write(const std::string&) = 0;
    virtual std::optional<std::string> readBack() = 0;
    virtual CommandResult verify(const std::string&) = 0;
    virtual LogAcquisitionResult acquireFlightLog(const std::string& logId) = 0;
    virtual CommandResult close() = 0;
};

} // namespace bluesky::autopilot
