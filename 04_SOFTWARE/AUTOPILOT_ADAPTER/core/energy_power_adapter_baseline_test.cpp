#include "energy_power_adapter_baseline.hpp"
#include <cassert>

using namespace bluesky::energy;

int main() {
    EnergyPowerAdapterBaseline adapter;
    assert(adapter.getSourceId() == "SRC-08");
    assert(!adapter.readSample().has_value());

    EnergySample sample;
    sample.sourceId = "SRC-08";
    sample.sourceTimestampMs = 1000;
    sample.voltageV = 24.0;
    sample.currentA = 10.0;
    sample.powerW = 240.0;
    sample.remainingPercent = 75.0;
    sample.consumedAh = 12.5;
    sample.estimatedRemainingSeconds = 1800.0;
    sample.state = EnergyState::Nominal;
    sample.quality = EnergyQuality::Valid;
    sample.valid = true;

    auto result = adapter.acceptSample(sample);
    assert(result.accepted);
    assert(adapter.getState() == EnergyState::Nominal);
    assert(adapter.getQuality() == EnergyQuality::Valid);
    assert(adapter.readSample()->remainingPercent == 75.0);

    sample.remainingPercent = 101.0;
    result = adapter.acceptSample(sample);
    assert(!result.accepted && result.error == EnergyError::InvalidData);

    sample.remainingPercent = 75.0;
    sample.stale = true;
    result = adapter.acceptSample(sample);
    assert(!result.accepted && result.error == EnergyError::StaleData);

    return 0;
}
