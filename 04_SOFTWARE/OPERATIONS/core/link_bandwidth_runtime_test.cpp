#include "link_bandwidth_runtime.hpp"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include <iostream>

using namespace bluesky::operations;

int main() {
    LinkBandwidthRuntime runtime;

    assert(!runtime.observeTransmit(0, 1000));
    assert(!runtime.observeReceive(10, 0));

    assert(runtime.observeTransmit(100, 1000));
    assert(runtime.observeTransmit(200, 1100));
    assert(runtime.observeReceive(80, 2000));
    assert(runtime.observeReceive(120, 2100));

    const auto snapshot = runtime.snapshot();
    assert(snapshot.transmitted_bytes == 300);
    assert(snapshot.received_bytes == 200);
    assert(snapshot.first_transmit_timestamp_ms == 1000);
    assert(snapshot.last_transmit_timestamp_ms == 1100);
    assert(snapshot.first_receive_timestamp_ms == 2000);
    assert(snapshot.last_receive_timestamp_ms == 2100);

    assert(snapshot.transmitRateBytesPerSecond());
    assert(*snapshot.transmitRateBytesPerSecond() == 3000.0);
    assert(snapshot.receiveRateBytesPerSecond());
    assert(*snapshot.receiveRateBytesPerSecond() == 2000.0);

    assert(!runtime.observeTransmit(10, 1099));
    assert(runtime.snapshot().transmitted_bytes == 300);

    runtime.reset();
    const auto reset = runtime.snapshot();
    assert(reset.transmitted_bytes == 0);
    assert(reset.received_bytes == 0);
    assert(!reset.transmitRateBytesPerSecond());
    assert(!reset.receiveRateBytesPerSecond());

    std::cout << "link_bandwidth_runtime_test: PASS\n";
}
