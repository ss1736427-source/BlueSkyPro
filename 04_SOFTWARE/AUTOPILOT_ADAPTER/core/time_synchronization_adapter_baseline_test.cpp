#include "time_synchronization_adapter_baseline.hpp"

#include <cassert>

using namespace bluesky::time_sync;

int main() {
    TimeSynchronizationAdapterBaseline adapter;

    TimeSyncSample sample;
    sample.sourceId = "SRC-12";
    sample.sourceTimestampMs = 1700000000000;
    sample.receiptTimestampMs = 1700000000008;
    sample.offsetMs = 1.7;
    sample.uncertaintyMs = 0.8;
    sample.state = TimeSyncState::Synchronized;
    sample.quality = TimeSyncQuality::Valid;
    sample.valid = true;

    auto accepted = adapter.acceptSample(sample);
    assert(accepted.accepted);
    assert(adapter.readSample().has_value());
    assert(adapter.getState() == TimeSyncState::Synchronized);

    auto invalid = sample;
    invalid.receiptTimestampMs = 0;
    auto invalidResult = adapter.acceptSample(invalid);
    assert(!invalidResult.accepted);
    assert(invalidResult.error == TimeSyncError::InvalidData);

    auto stale = sample;
    stale.stale = true;
    auto staleResult = adapter.acceptSample(stale);
    assert(!staleResult.accepted);
    assert(staleResult.error == TimeSyncError::StaleData);

    auto mismatch = sample;
    mismatch.sourceId = "SRC-OTHER";
    auto mismatchResult = adapter.acceptSample(mismatch);
    assert(!mismatchResult.accepted);
    assert(mismatchResult.error == TimeSyncError::ConfigurationMismatch);

    return 0;
}
