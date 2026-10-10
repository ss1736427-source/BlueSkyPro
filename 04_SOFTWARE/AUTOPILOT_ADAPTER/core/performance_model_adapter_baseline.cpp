#include "performance_model_adapter_baseline.hpp"

#include <cmath>
#include <utility>

namespace bluesky::performance {

PerformanceModelAdapterBaseline::PerformanceModelAdapterBaseline(std::string sourceId, std::string modelIdentity)
    : sourceId_(std::move(sourceId)), modelIdentity_(std::move(modelIdentity)) {}

std::string PerformanceModelAdapterBaseline::getSourceId() const { return sourceId_; }
std::string PerformanceModelAdapterBaseline::getModelIdentity() const { return modelIdentity_; }
std::string PerformanceModelAdapterBaseline::getConfigurationVersion() const { return "BASELINE-1"; }
PerformanceState PerformanceModelAdapterBaseline::getState() const {
    return sample_.has_value() ? sample_->state : PerformanceState::Unknown;
}
PerformanceQuality PerformanceModelAdapterBaseline::getQuality() const {
    return sample_.has_value() ? sample_->quality : PerformanceQuality::Unknown;
}
std::optional<PerformanceSample> PerformanceModelAdapterBaseline::readSample() const { return sample_; }

PerformanceResult PerformanceModelAdapterBaseline::acceptSample(const PerformanceSample& sample) {
    if (sourceId_.empty() || sample.sourceId != sourceId_) {
        return {false, PerformanceError::ConfigurationMismatch, "source identity mismatch"};
    }
    if (sample.sourceTimestampMs <= 0) {
        return {false, PerformanceError::InvalidData, "invalid source timestamp"};
    }
    if (!sample.valid) {
        return {false, PerformanceError::InvalidData, "sample marked invalid"};
    }
    if (sample.stale || sample.quality == PerformanceQuality::Stale) {
        return {false, PerformanceError::StaleData, "sample is stale"};
    }
    if (sample.quality == PerformanceQuality::Unknown || sample.state == PerformanceState::Unknown) {
        return {false, PerformanceError::InvalidData, "sample quality/state is unknown"};
    }

    const double values[] = {
        sample.predictedEnduranceSeconds, sample.predictedRangeMeters,
        sample.predictedEnergyPerMeterWh, sample.predictedPowerW,
        sample.observedEnduranceSeconds, sample.observedRangeMeters,
        sample.modelErrorPercent};
    for (double value : values) {
        if (!std::isfinite(value) || value < 0.0) {
            return {false, PerformanceError::InvalidData, "performance value out of range"};
        }
    }

    sample_ = sample;
    return {true, PerformanceError::None, "sample accepted"};
}

} // namespace bluesky::performance
