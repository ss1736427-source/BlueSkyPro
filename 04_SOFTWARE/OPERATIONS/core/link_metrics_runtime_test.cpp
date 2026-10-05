#include "link_metrics_runtime.hpp"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include <iostream>

using namespace bluesky::operations;

int main() {
    LinkMetricsRuntime metrics;

    assert(metrics.observe(254, 1000));
    assert(metrics.observe(255, 1010));
    assert(metrics.observe(0, 1020));
    assert(metrics.observe(2, 1040));

    auto snapshot = metrics.snapshot();
    assert(snapshot.observed_packets == 4);
    assert(snapshot.inferred_lost_packets == 1);
    assert(snapshot.packetLossRatio() == 0.2);
    assert(snapshot.last_sequence == 2);
    assert(snapshot.first_observation_timestamp_ms == 1000);
    assert(snapshot.last_observation_timestamp_ms == 1040);

    assert(metrics.observe(2, 1050));
    assert(metrics.observe(1, 1060));
    snapshot = metrics.snapshot();
    assert(snapshot.duplicate_packets == 1);
    assert(snapshot.out_of_order_packets == 1);
    assert(snapshot.observed_packets == 5);
    assert(snapshot.ageMs(1090).value() == 30);

    assert(!metrics.observe(3, 0));

    metrics.reset();
    snapshot = metrics.snapshot();
    assert(snapshot.observed_packets == 0);
    assert(!snapshot.sequence_initialized);
    assert(snapshot.packetLossRatio() == 0.0);
    assert(!snapshot.ageMs(1000).has_value());

    std::cout << "link_metrics_runtime_test: PASS\n";
}
