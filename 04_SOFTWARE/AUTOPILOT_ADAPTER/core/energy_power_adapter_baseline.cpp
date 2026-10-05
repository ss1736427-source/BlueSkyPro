#include "energy_power_adapter_baseline.hpp"
#include <utility>

namespace bluesky::energy {

EnergyPowerAdapterBaseline::EnergyPowerAdapterBaseline(std::string sourceId, std::string batteryIdentity)
    : sourceId_(std::move(sourceId)), batteryIdentity_(std::move(batteryIdentity)) {}

std::string EnergyPowerAdapterBaseline::getSourceId() const { return sourceId_; }
std::string EnergyPowerAdapterBaseline::getBatteryIdentity() const { return batteryIdentity_; }
std::string EnergyPowerAdapterBaseline::getConfigurationVersion() const { return "BASELINE-1"; }

EnergyState EnergyPowerAdapterBaseline::getState() const {
    return sample_ ? sample_->state : EnergyState::Unknown;
}

EnergyQuality EnergyPowerAdapterBaseline::getQuality() const {
    return sample_ ? sample_->quality : EnergyQuality::Unknown;
}

std::optional<EnergySample> EnergyPowerAdapterBaseline::readSample() const { return sample_; }

EnergyResult EnergyPowerAdapterBaseline::acceptSample(const EnergySample& sample) {
    if (sample.sourceId.empty() || sample.sourceId != sourceId_)
        return {false, EnergyError::ConfigurationMismatch, "source_id mismatch"};
    if (sample.sourceTimestampMs <= 0)
        return {false, EnergyError::InvalidData, "energy timestamp is required"};
    if (!sample.valid)
        return {false, EnergyError::InvalidData, "energy sample is invalid"};
    if (sample.stale)
        return {false, EnergyError::StaleData, "energy sample is stale"};
    if (sample.voltageV < 0.0 || sample.currentA < 0.0 || sample.powerW < 0.0 ||
        sample.remainingPercent < 0.0 || sample.remainingPercent > 100.0 ||
        sample.consumedAh < 0.0 || sample.estimatedRemainingSeconds < 0.0)
        return {false, EnergyError::InvalidData, "energy value out of range"};

    sample_ = sample;
    return {true, EnergyError::None, "accepted"};
}

} // namespace bluesky::energy
