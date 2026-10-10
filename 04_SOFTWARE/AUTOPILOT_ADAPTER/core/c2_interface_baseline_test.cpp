#include "c2_interface_baseline.hpp"

#include <cassert>
#include <iostream>

int main() {
    using namespace bluesky::c2;

    C2InterfaceBaseline c2({"UAV-001-C2", "LOOPBACK", "BLUESKY-C2", "1.0"});

    assert(c2.getIdentity().endpointId == "UAV-001-C2");
    assert(c2.getLinkState() == C2LinkState::Disconnected);

    const auto rejected = c2.send("TELEMETRY", "STATE=READY");
    assert(!rejected.accepted);
    assert(rejected.error == C2ErrorCode::NotConnected);

    assert(c2.connect());
    assert(c2.getLinkState() == C2LinkState::Connected);

    const auto auth = c2.authenticate("TEST-CREDENTIAL-REF");
    assert(auth.accepted);
    assert(c2.getLinkMetrics().healthy);

    const auto tx = c2.send("TELEMETRY", "STATE=READY");
    assert(tx.accepted);
    assert(tx.sequence != 0);

    const auto frame = c2.receive();
    assert(frame.has_value());
    assert(frame->sequence == tx.sequence);
    assert(frame->channel == "TELEMETRY");
    assert(frame->payload == "STATE=READY");

    const auto hb = c2.heartbeat(1000);
    assert(hb.accepted);
    assert(c2.getLinkMetrics().lastRxTimestampMs == 1000);
    assert(c2.getLinkMetrics().lastTxTimestampMs == 1000);

    const auto bad_auth = C2InterfaceBaseline({"UAV-002-C2", "LOOPBACK", "BLUESKY-C2", "1.0"})
                              .authenticate("");
    assert(!bad_auth.accepted);
    assert(bad_auth.error == C2ErrorCode::AuthenticationFailed);

    c2.disconnect();
    assert(c2.getLinkState() == C2LinkState::Disconnected);
    assert(!c2.getLinkMetrics().healthy);

    std::cout << "EC-01 C2 interface baseline: PASS\n";
    return 0;
}
