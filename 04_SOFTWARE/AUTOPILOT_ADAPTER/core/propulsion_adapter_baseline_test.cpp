#include "propulsion_adapter_baseline.hpp"

#include <cassert>

using namespace bluesky::propulsion;

int main() {
    PropulsionAdapterBaseline adapter;

    PropulsionSample sample;
    sample.sourceId = "SRC-09";
    sample.sourceTimestampMs = 1700000000000;
    sample.rpm = 4200.0;
    sample.thrustN = 38.0;
    sample.torqueNm = 1.8;
    sample.fuelFlowKgPerS = 0.004;
    sample.throttlePercent = 62.0;
    sample.temperatureC = 71.0;
    sample.vibrationRms = 0.12;
    sample.state = PropulsionState::Nominal;
    sample.quality = PropulsionQuality::Valid;
    sample.valid = true;

    auto accepted = adapter.acceptSample(sample);
    assert(accepted.accepted);
    assert(adapter.readSample().has_value());
    assert(adapter.getState() == PropulsionState::Nominal);
    assert(adapter.getQuality() == PropulsionQuality::Valid);

    auto invalid = sample;
    invalid.throttlePercent = 101.0;
    auto invalidResult = adapter.acceptSample(invalid);
    assert(!invalidResult.accepted);
    assert(invalidResult.error == PropulsionError::InvalidData);

    auto stale = sample;
    stale.stale = true;
    auto staleResult = adapter.acceptSample(stale);
    assert(!staleResult.accepted);
    assert(staleResult.error == PropulsionError::StaleData);

    auto mismatch = sample;
    mismatch.sourceId = "SRC-OTHER";
    auto mismatchResult = adapter.acceptSample(mismatch);
    assert(!mismatchResult.accepted);
    assert(mismatchResult.error == PropulsionError::ConfigurationMismatch);

    return 0;
}
