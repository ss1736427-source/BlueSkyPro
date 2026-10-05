#include "navigation_adapter_baseline.hpp"

#include <cassert>

using namespace bluesky::navigation;

int main() {
    NavigationAdapterBaseline adapter;

    assert(adapter.getSourceId() == "SRC-06");
    assert(!adapter.readSample().has_value());
    assert(adapter.getQuality() == NavigationQuality::Unknown);

    NavigationSample sample;
    sample.sourceId = "SRC-06";
    sample.timestampUtc = "2026-09-14T18:00:00Z";
    sample.sourceTimestampMs = 1726336800000;
    sample.adapterTimestampMs = 1726336800100;
    sample.position = {60.1699, 24.9384, 120.0};
    sample.velocity = {12.0, 1.5, 0.2};
    sample.attitude = {1.0, -2.0, 85.0};
    sample.fixType = FixType::RTKFixed;
    sample.quality = NavigationQuality::Usable;
    sample.correctionState = CorrectionState::Healthy;
    sample.satellites = 24;
    sample.horizontalAccuracyM = 0.02;
    sample.verticalAccuracyM = 0.04;
    sample.valid = true;
    sample.stale = false;

    const auto accepted = adapter.acceptSample(sample);
    assert(accepted.accepted);
    assert(accepted.error == NavigationError::None);

    const auto stored = adapter.readSample();
    assert(stored.has_value());
    assert(stored->sourceId == "SRC-06");
    assert(stored->fixType == FixType::RTKFixed);
    assert(stored->position.latitudeDeg == 60.1699);
    assert(stored->horizontalAccuracyM == 0.02);
    assert(adapter.getCorrectionState() == CorrectionState::Healthy);

    auto invalid = sample;
    invalid.valid = false;
    assert(!adapter.acceptSample(invalid).accepted);
    assert(adapter.acceptSample(invalid).error == NavigationError::InvalidData);

    auto stale = sample;
    stale.stale = true;
    assert(adapter.acceptSample(stale).error == NavigationError::StaleData);

    auto wrongSource = sample;
    wrongSource.sourceId = "SRC-02";
    assert(adapter.acceptSample(wrongSource).error == NavigationError::ConfigurationMismatch);

    return 0;
}
