#include "performance_model_adapter_baseline.hpp"

#include <cassert>

using namespace bluesky::performance;

int main() {
    PerformanceModelAdapterBaseline adapter;

    PerformanceSample sample;
    sample.sourceId = "SRC-10";
    sample.sourceTimestampMs = 1700000000000;
    sample.predictedEnduranceSeconds = 7200.0;
    sample.predictedRangeMeters = 180000.0;
    sample.predictedEnergyPerMeterWh = 0.004;
    sample.predictedPowerW = 720.0;
    sample.observedEnduranceSeconds = 7000.0;
    sample.observedRangeMeters = 174000.0;
    sample.modelErrorPercent = 2.86;
    sample.state = PerformanceState::Nominal;
    sample.quality = PerformanceQuality::Valid;
    sample.valid = true;

    auto accepted = adapter.acceptSample(sample);
    assert(accepted.accepted);
    assert(adapter.readSample().has_value());
    assert(adapter.getState() == PerformanceState::Nominal);
    assert(adapter.getQuality() == PerformanceQuality::Valid);

    auto invalid = sample;
    invalid.predictedPowerW = -1.0;
    auto invalidResult = adapter.acceptSample(invalid);
    assert(!invalidResult.accepted);
    assert(invalidResult.error == PerformanceError::InvalidData);

    auto stale = sample;
    stale.stale = true;
    auto staleResult = adapter.acceptSample(stale);
    assert(!staleResult.accepted);
    assert(staleResult.error == PerformanceError::StaleData);

    auto mismatch = sample;
    mismatch.sourceId = "SRC-OTHER";
    auto mismatchResult = adapter.acceptSample(mismatch);
    assert(!mismatchResult.accepted);
    assert(mismatchResult.error == PerformanceError::ConfigurationMismatch);

    return 0;
}
