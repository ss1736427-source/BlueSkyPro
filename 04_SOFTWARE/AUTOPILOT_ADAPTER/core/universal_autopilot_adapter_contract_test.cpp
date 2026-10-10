#include "universal_autopilot_adapter.hpp"
#include "../../PLANNING/model/mission_model.hpp"
#include "../../PLANNING/model/vehicle_equipment_capability.hpp"
#include <cassert>
#include <type_traits>
namespace {
class ContractAdapter final : public bluesky::autopilot::UniversalAutopilotAdapter {
public:
    bluesky::autopilot::Identity getVehicleIdentity() const override { return {}; }
    std::string getAutopilotIdentity() const override { return {}; }
    std::string getFirmwareVersion() const override { return {}; }
    std::string getProtocolVersion() const override { return {}; }
    bluesky::autopilot::Capabilities getCapabilities() const override { return {}; }
    std::vector<std::string> getSupportedCommands() const override { return {}; }
    std::vector<std::string> getSupportedMissionFeatures() const override { return {}; }
    std::vector<std::string> getSupportedEquipmentFeatures() const override { return {}; }
    bool connect() override { return true; }
    void disconnect() override {}
    bluesky::autopilot::ConnectionState getConnectionState() const override { return bluesky::autopilot::ConnectionState::Connected; }
    bluesky::autopilot::LinkMetrics getLinkMetrics() const override { return {}; }
    bluesky::autopilot::NormalizedState getNavigationState() const override { return {}; }
    bluesky::autopilot::NormalizedState getFlightMode() const override { return {}; }
    bluesky::autopilot::NormalizedState getHealthState() const override { return {}; }
    bluesky::autopilot::NormalizedState getFailsafeState() const override { return {}; }
    bluesky::autopilot::NormalizedState getEnergyState() const override { return {}; }
    bluesky::autopilot::NormalizedState getMissionState() const override { return {}; }
    bluesky::autopilot::NormalizedState getEquipmentState() const override { return {}; }
    bluesky::autopilot::CommandResult arm() override { return {}; }
    bluesky::autopilot::CommandResult disarm() override { return {}; }
    bluesky::autopilot::CommandResult takeoff(double) override { return {}; }
    bluesky::autopilot::CommandResult land() override { return {}; }
    bluesky::autopilot::CommandResult hold() override { return {}; }
    bluesky::autopilot::CommandResult resume() override { return {}; }
    bluesky::autopilot::CommandResult rtl() override { return {}; }
    bluesky::autopilot::CommandResult gotoPosition(double, double, double) override { return {}; }
    bluesky::autopilot::CommandResult startMission() override { return {}; }
    bluesky::autopilot::CommandResult pauseMission() override { return {}; }
    bluesky::autopilot::CommandResult resumeMission() override { return {}; }
    bluesky::autopilot::CommandResult abortMission() override { return {}; }
    std::optional<std::string> compileMission(const bluesky::planning::Mission&, const bluesky::planning::VehicleEquipmentCapabilityProfile&) override { return std::string{"compiled"}; }
    bool uploadMission(const std::string&) override { return true; }
    std::optional<std::string> downloadMission() override { return std::string{"downloaded"}; }
    std::optional<std::string> readBackMission() override { return std::string{"read-back"}; }
    bluesky::autopilot::MissionComparison compareMission(const std::string&, const std::string&) const override { return {true, {}}; }
    std::optional<std::string> readBaseline() override { return std::string{"baseline"}; }
    bluesky::autopilot::MissionComparison compare(const std::string&, const std::string&) const override { return {true, {}}; }
    bluesky::autopilot::CommandResult validate(const std::string&) const override { return {}; }
    bluesky::autopilot::CommandResult write(const std::string&) override { return {}; }
    std::optional<std::string> readBack() override { return std::string{"read-back"}; }
    bluesky::autopilot::CommandResult verify(const std::string&) override { return {}; }
    bluesky::autopilot::LogAcquisitionResult acquireFlightLog(const std::string&) override { return {}; }
    bluesky::autopilot::CommandResult close() override { return {}; }
};
}
int main() {
    using bluesky::autopilot::ConnectionState;
    using bluesky::autopilot::ExecutionState;
    using bluesky::autopilot::UniversalAutopilotAdapter;
    static_assert(std::is_abstract_v<UniversalAutopilotAdapter>);
    const ExecutionState executionStates[] = {ExecutionState::Requested, ExecutionState::Validating, ExecutionState::Rejected, ExecutionState::Dispatched, ExecutionState::Acknowledged, ExecutionState::Executing, ExecutionState::Completed, ExecutionState::Failed, ExecutionState::Cancelled, ExecutionState::Timeout, ExecutionState::Unknown};
    for (std::size_t i = 0; i < std::size(executionStates); ++i) for (std::size_t j = i + 1; j < std::size(executionStates); ++j) assert(executionStates[i] != executionStates[j]);
    ContractAdapter adapter;
    const bluesky::planning::Mission mission{};
    const bluesky::planning::VehicleEquipmentCapabilityProfile capabilities{};
    assert(adapter.getConnectionState() == ConnectionState::Connected);
    assert(adapter.getAutopilotIdentity().empty());
    assert(adapter.compileMission(mission, capabilities).has_value());
    assert(adapter.readBackMission().has_value());
    assert(adapter.readBaseline().has_value());
    assert(adapter.compare("baseline", "baseline").equal);
    assert(!adapter.acquireFlightLog("LOG-024-CONTRACT").accepted);
    assert(adapter.close().error == bluesky::autopilot::ErrorCode::None);
    return 0;
}
