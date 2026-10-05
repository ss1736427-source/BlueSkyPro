#include "propulsion_adapter_baseline.hpp"

namespace bluesky::propulsion {

PropulsionAdapterBaseline::PropulsionAdapterBaseline(std::string sourceId, std::string propulsionIdentity)
    : sourceId_(std::move(sourceId)), propulsionIdentity_(std::move(propulsionIdentity)) {}

std::string PropulsionAdapterBaseline::getSourceId() const { return sourceId_; }
std::string PropulsionAdapterBaseline::getPropulsionIdentity() const { return propulsionIdentity_; }
std::string PropulsionAdapterBaseline::getConfigurationVersion() const { return "BASELINE-1"; }
PropulsionState PropulsionAdapterBaseline::getState() const {
    return sample_.has_value() ? sample_->state : PropulsionState::Unknown;
}
PropulsionQuality PropulsionAdapterBaseline::getQuality() const {
    return sample_.has_value() ? sample_->quality : PropulsionQuality::Unknown;
}
std::optional<PropulsionSample> PropulsionAdapterBaseline::readSample() const { return sample_; }

PropulsionResult PropulsionAdapterBaseline::acceptSample(const PropulsionSample& sample) {
    if (sourceId_.empty() || sample.sourceId != sourceId_) {
        return {false, PropulsionError::ConfigurationMismatch, "source identity mismatch"};
    }
    if (sample.sourceTimestampMs <= 0) {
        return {false, PropulsionError::InvalidData, "invalid source timestamp"};
    }
    if (!sample.valid) {
        return {false, PropulsionError::InvalidData, "sample marked invalid"};
    }
    if (sample.stale || sample.quality == PropulsionQuality::Stale) {
        return {false, PropulsionError::StaleData, "sample is stale"};
    }
    if (sample.quality == PropulsionQuality::Unknown || sample.state == PropulsionState::Unknown) {
        return {false, PropulsionError::InvalidData, "sample quality/state is unknown"};
    }
    if (sample.rpm < 0.0 || sample.thrustN < 0.0 || sample.fuelFlowKgPerS < 0.0 ||
        sample.throttlePercent < 0.0 || sample.throttlePercent > 100.0 || sample.temperatureC < -100.0 ||
        sample.vibrationRms < 0.0) {
        return {false, PropulsionError::InvalidData, "propulsion value out of range"};
    }

    sample_ = sample;
    return {true, PropulsionError::None, "sample accepted"};
}

} // namespace bluesky::propulsion
