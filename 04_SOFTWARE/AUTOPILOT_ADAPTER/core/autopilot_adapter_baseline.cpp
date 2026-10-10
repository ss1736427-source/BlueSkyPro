#include "autopilot_adapter_baseline.hpp"
#include "../../PLANNING/model/mission_model.hpp"
#include "../../PLANNING/model/vehicle_equipment_capability.hpp"
#include <utility>
namespace bluesky::autopilot {
AutopilotAdapterBaseline::AutopilotAdapterBaseline(Identity identity) : identity_(std::move(identity)) {}
Identity AutopilotAdapterBaseline::getVehicleIdentity() const { return identity_; }
std::string AutopilotAdapterBaseline::getAutopilotIdentity() const { return identity_.autopilot; }
std::string AutopilotAdapterBaseline::getFirmwareVersion() const { return identity_.firmwareVersion; }
std::string AutopilotAdapterBaseline::getProtocolVersion() const { return identity_.protocolVersion; }
Capabilities AutopilotAdapterBaseline::getCapabilities() const { return {{"arm", "disarm", "takeoff", "land", "hold", "resume", "rtl", "gotoPosition", "startMission", "pauseMission", "resumeMission", "abortMission"}, {"mission_upload", "mission_download", "mission_readback", "mission_compare", "flight_log_acquisition", "clean_close"}, {}}; }
std::vector<std::string> AutopilotAdapterBaseline::getSupportedCommands() const { return getCapabilities().commands; }
std::vector<std::string> AutopilotAdapterBaseline::getSupportedMissionFeatures() const { return getCapabilities().missionFeatures; }
std::vector<std::string> AutopilotAdapterBaseline::getSupportedEquipmentFeatures() const { return getCapabilities().equipmentFeatures; }
bool AutopilotAdapterBaseline::connect() { closed_ = false; sessionStarted_ = true; connectionState_ = ConnectionState::Connected; linkMetrics_.healthy = true; return true; }
void AutopilotAdapterBaseline::disconnect() { connectionState_ = ConnectionState::Disconnected; linkMetrics_.healthy = false; }
ConnectionState AutopilotAdapterBaseline::getConnectionState() const { return connectionState_; }
LinkMetrics AutopilotAdapterBaseline::getLinkMetrics() const { return linkMetrics_; }
NormalizedState AutopilotAdapterBaseline::getNavigationState() const { return {"UNKNOWN"}; }
NormalizedState AutopilotAdapterBaseline::getFlightMode() const { return {"UNKNOWN"}; }
NormalizedState AutopilotAdapterBaseline::getHealthState() const { return {"UNKNOWN"}; }
NormalizedState AutopilotAdapterBaseline::getFailsafeState() const { return {"UNKNOWN"}; }
NormalizedState AutopilotAdapterBaseline::getEnergyState() const { return {"UNKNOWN"}; }
NormalizedState AutopilotAdapterBaseline::getMissionState() const { return {mission_.empty() ? "EMPTY" : "LOADED"}; }
NormalizedState AutopilotAdapterBaseline::getEquipmentState() const { return {"UNKNOWN"}; }
CommandResult AutopilotAdapterBaseline::command(const std::string& name) { CommandResult result; result.commandId = identity_.vehicleId + "-CMD-" + std::to_string(++commandSequence_); result.vehicleId = identity_.vehicleId; result.executionState = ExecutionState::Requested; if (connectionState_ != ConnectionState::Connected || closed_) { result.executionState = ExecutionState::Rejected; result.error = ErrorCode::NotConnected; result.reason = "AUTOPILOT_NOT_CONNECTED"; return result; } result.executionState = ExecutionState::Acknowledged; result.acknowledged = true; result.reason = name; return result; }
CommandResult AutopilotAdapterBaseline::arm() { return command("arm"); }
CommandResult AutopilotAdapterBaseline::disarm() { return command("disarm"); }
CommandResult AutopilotAdapterBaseline::takeoff(double) { return command("takeoff"); }
CommandResult AutopilotAdapterBaseline::land() { return command("land"); }
CommandResult AutopilotAdapterBaseline::hold() { return command("hold"); }
CommandResult AutopilotAdapterBaseline::resume() { return command("resume"); }
CommandResult AutopilotAdapterBaseline::rtl() { return command("rtl"); }
CommandResult AutopilotAdapterBaseline::gotoPosition(double, double, double) { return command("gotoPosition"); }
CommandResult AutopilotAdapterBaseline::startMission() { return command("startMission"); }
CommandResult AutopilotAdapterBaseline::pauseMission() { return command("pauseMission"); }
CommandResult AutopilotAdapterBaseline::resumeMission() { return command("resumeMission"); }
CommandResult AutopilotAdapterBaseline::abortMission() { return command("abortMission"); }
std::optional<std::string> AutopilotAdapterBaseline::compileMission(const bluesky::planning::Mission& mission, const bluesky::planning::VehicleEquipmentCapabilityProfile& capabilities) { (void)capabilities; if (mission.identity.mission_id.empty() || identity_.vehicleId.empty()) return std::nullopt; mission_ = mission.identity.mission_id + ":" + identity_.vehicleId; return mission_; }
bool AutopilotAdapterBaseline::uploadMission(const std::string& compiledMission) { if (connectionState_ != ConnectionState::Connected || compiledMission.empty() || closed_) return false; mission_ = compiledMission; return true; }
std::optional<std::string> AutopilotAdapterBaseline::downloadMission() { if (connectionState_ != ConnectionState::Connected || mission_.empty() || closed_) return std::nullopt; return mission_; }
std::optional<std::string> AutopilotAdapterBaseline::readBackMission() { return downloadMission(); }
MissionComparison AutopilotAdapterBaseline::compareMission(const std::string& approvedMission, const std::string& actualMission) const { return {approvedMission == actualMission, approvedMission == actualMission ? std::vector<std::string>{} : std::vector<std::string>{"MISSION_CONTENT_MISMATCH"}}; }
std::optional<std::string> AutopilotAdapterBaseline::readBaseline() { if (configuration_.empty()) return std::nullopt; return configuration_; }
MissionComparison AutopilotAdapterBaseline::compare(const std::string& baseline, const std::string& actual) const { return compareMission(baseline, actual); }
CommandResult AutopilotAdapterBaseline::validate(const std::string& configuration) const { CommandResult result; result.vehicleId = identity_.vehicleId; result.executionState = configuration.empty() ? ExecutionState::Rejected : ExecutionState::Acknowledged; result.acknowledged = !configuration.empty(); result.error = configuration.empty() ? ErrorCode::InvalidState : ErrorCode::None; result.reason = configuration.empty() ? "CONFIGURATION_REQUIRED" : "VALID"; return result; }
CommandResult AutopilotAdapterBaseline::write(const std::string& configuration) { auto result = validate(configuration); if (result.acknowledged) configuration_ = configuration; return result; }
std::optional<std::string> AutopilotAdapterBaseline::readBack() { return readBaseline(); }
CommandResult AutopilotAdapterBaseline::verify(const std::string& baseline) { auto result = validate(baseline); if (!result.acknowledged) return result; const auto actual = readBack(); result.acknowledged = actual.has_value() && actual.value() == baseline; result.executionState = result.acknowledged ? ExecutionState::Completed : ExecutionState::Failed; result.error = result.acknowledged ? ErrorCode::None : ErrorCode::ConfigurationMismatch; result.reason = result.acknowledged ? "VERIFIED" : "CONFIGURATION_MISMATCH"; return result; }
LogAcquisitionResult AutopilotAdapterBaseline::acquireFlightLog(const std::string& logId) { LogAcquisitionResult result; result.logId = logId; result.vehicleId = identity_.vehicleId; if (connectionState_ != ConnectionState::Connected || closed_) { result.executionState = ExecutionState::Rejected; result.error = ErrorCode::NotConnected; result.reason = "AUTOPILOT_NOT_CONNECTED"; return result; } if (logId.empty()) { result.executionState = ExecutionState::Rejected; result.error = ErrorCode::InvalidState; result.reason = "LOG_ID_REQUIRED"; return result; } result.accepted = true; result.executionState = ExecutionState::Completed; result.reason = "ACQUIRED"; result.sourceReference = "SIL_LOG_SOURCE:" + logId; return result; }
CommandResult AutopilotAdapterBaseline::close() { CommandResult result; result.commandId = identity_.vehicleId + "-CLOSE-" + std::to_string(++commandSequence_); result.vehicleId = identity_.vehicleId; if (closed_) { result.executionState = ExecutionState::Completed; result.acknowledged = true; result.reason = "ALREADY_CLOSED"; return result; } if (!sessionStarted_) { result.executionState = ExecutionState::Rejected; result.error = ErrorCode::InvalidState; result.reason = "NO_OPERATIONAL_LIFECYCLE"; return result; } closed_ = true; connectionState_ = ConnectionState::Disconnected; linkMetrics_.healthy = false; result.executionState = ExecutionState::Completed; result.acknowledged = true; result.reason = "CLOSED_AND_ARCHIVE_READY"; return result; }
} // namespace bluesky::autopilot
