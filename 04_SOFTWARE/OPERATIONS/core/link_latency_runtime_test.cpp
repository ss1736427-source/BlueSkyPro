#include "link_latency_runtime.hpp"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include <iostream>

using namespace bluesky::operations;

int main() {
    LinkLatencyRuntime r;

    assert(!r.observeProbeSent(0, 1000000));
    assert(r.observeProbeSent(1000, 1000000));
    assert(r.observeProbeSent(1100, 2000000));

    assert(!r.observeTimesyncResponse(1050, 999999));
    auto s = r.snapshot();
    assert(s.probes_sent == 2);
    assert(s.responses_received == 0);
    assert(s.unmatched_responses == 1);

    assert(r.observeTimesyncResponse(1055, 1000000));
    assert(r.observeTimesyncResponse(1175, 2000000));
    s = r.snapshot();
    assert(s.responses_received == 2);
    assert(s.last_rtt_ms && *s.last_rtt_ms == 75);
    assert(s.min_rtt_ms && *s.min_rtt_ms == 55);
    assert(s.max_rtt_ms && *s.max_rtt_ms == 75);
    assert(s.last_measurement_timestamp_ms == 1175);

    assert(!r.observeTimesyncResponse(1200, 1000000));
    r.reset();
    s = r.snapshot();
    assert(s.probes_sent == 0 && s.responses_received == 0);
    assert(!s.last_rtt_ms);

    std::cout << "link_latency_runtime_test: PASS\n";
}
