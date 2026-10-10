#pragma once

#include "universal_energy_power_adapter.hpp"

namespace bluesky::energy {

class EnergyPowerAdapterBaseline final : public UniversalEnergyPowerAdapter {
public:
    explicit EnergyPowerAdapterBaseline(std::string sourceId = "SRC-08", std::string batteryIdentity = "DEMO-BATTERY");

    std::string getSourceId() const override;
    std::string getBatteryIdentity() const override;
    std::string getConfigurationVersion() const override;
    EnergyState getState() const override;
    EnergyQuality getQuality() const override;
    std::optional<EnergySample> readSample() const override;
    EnergyResult acceptSample(const EnergySample& sample) override;

private:
    std::string sourceId_;
    std::string batteryIdentity_;
    std::optional<EnergySample> sample_;
};

} // namespace bluesky::energy
