#include "navigation_adapter_baseline.hpp"

#include <stdexcept>

namespace bluesky::navigation {

NavigationAdapterBaseline::NavigationAdapterBaseline(std::string sourceId)
    : sourceId_(std::move(sourceId)) {}

std::string NavigationAdapterBaseline::getSourceId() const { return sourceId_; }
std::string NavigationAdapterBaseline::getReceiverIdentity() const { return "DEMO-GNSS-RTK-NTRIP"; }
std::string NavigationAdapterBaseline::getFirmwareVersion() const { return "DEMO"; }
std::string NavigationAdapterBaseline::getProtocolVersion() const { return "BASELINE-1"; }

NavigationQuality NavigationAdapterBaseline::getQuality() const {
    return hasSample_ ? sample_.quality : NavigationQuality::Unknown;
}

CorrectionState NavigationAdapterBaseline::getCorrectionState() const {
    return hasSample_ ? sample_.correctionState : CorrectionState::Unknown;
}

std::optional<NavigationSample> NavigationAdapterBaseline::readSample() const {
    if (!hasSample_) return std::nullopt;
    return sample_;
}

NavigationResult NavigationAdapterBaseline::acceptSample(const NavigationSample& sample) {
    if (sample.sourceId.empty() || sample.sourceId != sourceId_)
        return {false, NavigationError::ConfigurationMismatch, "source_id mismatch"};
    if (sample.timestampUtc.empty() || sample.sourceTimestampMs <= 0)
        return {false, NavigationError::InvalidData, "navigation timestamp is required"};
    if (!sample.valid)
        return {false, NavigationError::InvalidData, "navigation sample is invalid"};
    if (sample.stale)
        return {false, NavigationError::StaleData, "navigation sample is stale"};

    sample_ = sample;
    hasSample_ = true;
    return {true, NavigationError::None, "accepted"};
}

} // namespace bluesky::navigation
