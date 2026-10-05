#include "time_synchronization_runtime.hpp"

#include <cassert>

using namespace bluesky::operations;

int main() {
    TimeSynchronizationRuntime runtime;

    assert(runtime.snapshot().state == TimeSynchronizationState::LocalOnly);
    assert(runtime.observe(1000, 1060, 2030));

    const auto synchronized = runtime.snapshot();
    assert(synchronized.state == TimeSynchronizationState::Synchronized);
    assert(synchronized.observations == 1);
    assert(synchronized.offset_ms.has_value());
    assert(*synchronized.offset_ms == 2000);
    assert(synchronized.last_rtt_ms.has_value());
    assert(*synchronized.last_rtt_ms == 60);
    assert(synchronized.last_observation_timestamp_ms == 1060);

    assert(!runtime.observe(0, 1060, 2030));
    assert(!runtime.observe(1100, 1090, 2030));
    assert(!runtime.observe(1100, 1160, 0));
    assert(runtime.snapshot().observations == 1);

    runtime.markStale(2000, 500);
    assert(runtime.snapshot().state == TimeSynchronizationState::Stale);

    runtime.markStale(2000, 500);
    assert(runtime.snapshot().state == TimeSynchronizationState::Stale);

    assert(runtime.observe(3000, 3040, 5050));
    assert(runtime.snapshot().state == TimeSynchronizationState::Synchronized);
    assert(*runtime.snapshot().offset_ms == 2030);
    assert(*runtime.snapshot().last_rtt_ms == 40);

    runtime.reset();
    assert(runtime.snapshot().state == TimeSynchronizationState::LocalOnly);
    assert(runtime.snapshot().observations == 0);
    return 0;
}
