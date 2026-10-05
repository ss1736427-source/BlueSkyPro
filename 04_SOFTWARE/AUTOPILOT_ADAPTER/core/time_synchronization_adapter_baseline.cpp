#include "time_synchronization_adapter_baseline.hpp"

#include <cmath>
#include <utility>

namespace bluesky::time_sync {

TimeSynchronizationAdapterBaseline::TimeSynchronizationAdapterBaseline(std::string sourceId, std::string timeReferenceIdentity)
    : sourceId_(std::move(sourceId)), timeReferenceIdentity_(std::move(timeReferenceIdentity)) {}

std::string TimeSynchronizationAdapterBaseline::getSourceId() const { return sourceId_; }
std::string TimeSynchronizationAdapterBaseline::getTimeReferenceIdentity() const { return timeReferenceIdentity_; }
std::string TimeSynchronizationAdapterBaseline::getConfigurationVersion() const { return "BASELINE-1"; }
TimeSyncState TimeSynchronizationAdapterBaseline::getState() const {
    return sample_.has_value() ? sample_->state : TimeSyncState::Unknown;
}
TimeSyncQuality TimeSynchronizationAdapterBaseline::getQuality() const {
    return sample_.has_value() ? sample_->quality : TimeSyncQuality::Unknown;
}
std::optional<TimeSyncSample> TimeSynchronizationAdapterBaseline::readSample() const { return sample_; }

TimeSyncResult TimeSynchronizationAdapterBaseline::acceptSample(const TimeSyncSample& sample) {
    if (sourceId_.empty() || sample.sourceId != sourceId_) {
        return {false, TimeSyncError::ConfigurationMismatch, "source identity mismatch"};
    }
    if (sample.sourceTimestampMs <= 0 || sample.receiptTimestampMs <= 0) {
        return {false, TimeSyncError::InvalidData, "invalid timestamp"};
    }
    if (!sample.valid) {
        return {false, TimeSyncError::InvalidData, "sample marked invalid"};
    }
    if (sample.stale || sample.quality == TimeSyncQuality::Stale) {
        return {false, TimeSyncError::StaleData, "sample is stale"};
    }
    if (sample.quality == TimeSyncQuality::Unknown || sample.state == TimeSyncState::Unknown) {
        return {false, TimeSyncError::InvalidData, "time synchronization state/quality is unknown"};
    }
    if (!std::isfinite(sample.offsetMs) || !std::isfinite(sample.uncertaintyMs) || sample.uncertaintyMs < 0.0) {
        return {false, TimeSyncError::InvalidData, "time synchronization value out of range"};
    }

    sample_ = sample;
    return {true, TimeSyncError::None, "sample accepted"};
}

} // namespace bluesky::time_sync
