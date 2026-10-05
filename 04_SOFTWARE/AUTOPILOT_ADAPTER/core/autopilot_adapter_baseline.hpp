#pragma once
#include "universal_autopilot_adapter.hpp"
#include <cstdint>
#include <optional>
#include <string>
namespace bluesky::autopilot {
class AutopilotAdapterBaseline : public UniversalAutopilotAdapter {
public:
    explicit AutopilotAdapterBaseline(Identity identity = {});
    Identity getVehicleIdentity() const override;
    std::string getAutopilotIdentity() const override;
    std::string getFirmwareVersion() const override;
    std::string getProtocolVersion() const override;
    Capabilities getCapabilities() const override;
    std::vector<std::string> getSupportedCommands() const override;
    std::vector<std::string> getSupportedMissionFeatures() const override;
    std::vector<std::string> getSupportedEquipmentFeatures() const override;
    bool connect() override;
    void disconnect() override;
    ConnectionState getConnectionState() const override;
    LinkMetrics getLinkMetrics() const override;
    NormalizedState getNavigationState() const override;
    NormalizedState getFlightMode() const override;
    NormalizedState getHealthState() const override;
    NormalizedState getFailsafeState() const override;
    NormalizedState getEnergyState() const override;
    NormalizedState getMissionState() const override;
    NormalizedState getEquipmentState() const override;
    CommandResult arm() override;
    CommandResult disarm() override;
    CommandResult takeoff(double altitudeM) override;
    CommandResult land() override;
    CommandResult hold() override;
    CommandResult resume() override;
    CommandResult rtl() override;
    CommandResult gotoPosition(double latDeg, double lonDeg, double altitudeM) override;
    CommandResult startMission() override;
    CommandResult pauseMission() override;
    CommandResult resumeMission() override;
    CommandResult abortMission() override;
    std::optional<std::string> compileMission(const bluesky::planning::Mission& mission, const bluesky::planning::VehicleEquipmentCapabilityProfile& capabilities) override;
    bool uploadMission(const std::string& compiledMission) override;
    std::optional<std::string> downloadMission() override;
    std::optional<std::string> readBackMission() override;
    MissionComparison compareMission(const std::string& approvedMission, const std::string& actualMission) const override;
    std::optional<std::string> readBaseline() override;
    MissionComparison compare(const std::string& baseline, const std::string& actual) const override;
    CommandResult validate(const std::string& configuration) const override;
    CommandResult write(const std::string& configuration) override;
    std::optional<std::string> readBack() override;
    CommandResult verify(const std::string& baseline) override;
    LogAcquisitionResult acquireFlightLog(const std::string& logId) override;
    CommandResult close() override;
private:
    CommandResult command(const std::string& name);
    Identity identity_;
    ConnectionState connectionState_{ConnectionState::Disconnected};
    LinkMetrics linkMetrics_{};
    std::string mission_{};
    std::string configuration_{};
    std::uint64_t commandSequence_{0};
    bool closed_{false};
    bool sessionStarted_{false};
};
}
