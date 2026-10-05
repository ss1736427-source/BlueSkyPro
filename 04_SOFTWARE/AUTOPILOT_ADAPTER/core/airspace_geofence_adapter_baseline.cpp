#include "airspace_geofence_adapter_baseline.hpp"

#include <cmath>
#include <utility>

namespace bluesky::airspace {

AirspaceGeofenceAdapterBaseline::AirspaceGeofenceAdapterBaseline(std::string sourceId, std::string airspaceIdentity)
    : sourceId_(std::move(sourceId)), airspaceIdentity_(std::move(airspaceIdentity)) {}

std::string AirspaceGeofenceAdapterBaseline::getSourceId() const { return sourceId_; }
std::string AirspaceGeofenceAdapterBaseline::getAirspaceIdentity() const { return airspaceIdentity_; }
std::string AirspaceGeofenceAdapterBaseline::getConfigurationVersion() const { return "BASELINE-1"; }
AirspaceState AirspaceGeofenceAdapterBaseline::getState() const {
    return sample_.has_value() ? sample_->state : AirspaceState::Unknown;
}
AirspaceQuality AirspaceGeofenceAdapterBaseline::getQuality() const {
    return sample_.has_value() ? sample_->quality : AirspaceQuality::Unknown;
}
std::optional<AirspaceSample> AirspaceGeofenceAdapterBaseline::readSample() const { return sample_; }

AirspaceResult AirspaceGeofenceAdapterBaseline::acceptSample(const AirspaceSample& sample) {
    if (sourceId_.empty() || sample.sourceId != sourceId_) {
        return {false, AirspaceError::ConfigurationMismatch, "source identity mismatch"};
    }
    if (sample.sourceTimestampMs <= 0) {
        return {false, AirspaceError::InvalidData, "invalid source timestamp"};
    }
    if (!sample.valid) {
        return {false, AirspaceError::InvalidData, "sample marked invalid"};
    }
    if (sample.stale || sample.quality == AirspaceQuality::Stale) {
        return {false, AirspaceError::StaleData, "sample is stale"};
    }
    if (sample.quality == AirspaceQuality::Unknown || sample.state == AirspaceState::Unknown ||
        sample.decision == AirspaceDecision::Unknown) {
        return {false, AirspaceError::InvalidData, "airspace decision/state/quality is unknown"};
    }
    if (!std::isfinite(sample.latitudeDeg) || sample.latitudeDeg < -90.0 || sample.latitudeDeg > 90.0 ||
        !std::isfinite(sample.longitudeDeg) || sample.longitudeDeg < -180.0 || sample.longitudeDeg > 180.0 ||
        !std::isfinite(sample.altitudeM) || sample.altitudeM < 0.0) {
        return {false, AirspaceError::InvalidData, "geographic value out of range"};
    }
    if (sample.insideRestriction && sample.decision == AirspaceDecision::Allowed && !sample.clearancePresent) {
        return {false, AirspaceError::InvalidData, "restricted position marked allowed without clearance"};
    }

    sample_ = sample;
    return {true, AirspaceError::None, "sample accepted"};
}

} // namespace bluesky::airspace
