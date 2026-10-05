#include "airspace_geofence_adapter_baseline.hpp"

#include <cassert>

using namespace bluesky::airspace;

int main() {
    AirspaceGeofenceAdapterBaseline adapter;

    AirspaceSample sample;
    sample.sourceId = "SRC-11";
    sample.sourceTimestampMs = 1700000000000;
    sample.latitudeDeg = 59.9386;
    sample.longitudeDeg = 30.3141;
    sample.altitudeM = 120.0;
    sample.insideRestriction = false;
    sample.clearancePresent = false;
    sample.decision = AirspaceDecision::Allowed;
    sample.state = AirspaceState::Clear;
    sample.quality = AirspaceQuality::Valid;
    sample.valid = true;

    auto accepted = adapter.acceptSample(sample);
    assert(accepted.accepted);
    assert(adapter.readSample().has_value());
    assert(adapter.getState() == AirspaceState::Clear);

    auto invalid = sample;
    invalid.latitudeDeg = 91.0;
    auto invalidResult = adapter.acceptSample(invalid);
    assert(!invalidResult.accepted);
    assert(invalidResult.error == AirspaceError::InvalidData);

    auto stale = sample;
    stale.stale = true;
    auto staleResult = adapter.acceptSample(stale);
    assert(!staleResult.accepted);
    assert(staleResult.error == AirspaceError::StaleData);

    auto mismatch = sample;
    mismatch.sourceId = "SRC-OTHER";
    auto mismatchResult = adapter.acceptSample(mismatch);
    assert(!mismatchResult.accepted);
    assert(mismatchResult.error == AirspaceError::ConfigurationMismatch);

    auto unsafe = sample;
    unsafe.insideRestriction = true;
    unsafe.clearancePresent = false;
    auto unsafeResult = adapter.acceptSample(unsafe);
    assert(!unsafeResult.accepted);
    assert(unsafeResult.error == AirspaceError::InvalidData);

    return 0;
}
