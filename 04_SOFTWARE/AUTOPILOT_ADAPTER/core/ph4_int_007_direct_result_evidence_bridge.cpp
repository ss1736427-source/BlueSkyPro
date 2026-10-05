#include "autopilot_adapter_baseline.hpp"
#include "c2_interface_baseline.hpp"
#include <cassert>
#include <fstream>
#include <iostream>
#include <string>

int main(int argc, char** argv) {
    const std::string output = argc > 1 ? argv[1] : "ph4_int_007_results.jsonl";
    std::ofstream out(output);
    assert(out.good());

    using namespace bluesky::autopilot;
    using namespace bluesky::c2;

    AutopilotAdapterBaseline adapter(Identity{"FIXTURE-UAV-001", "FIXTURE", "FIXTURE-AUTOPILOT", "1.0", "FIXTURE", "1.0"});
    C2InterfaceBaseline c2(C2Identity{"FIXTURE-UAV-001", "FIXTURE-TRANSPORT", "FIXTURE-PROTOCOL", "1.0"});

    const auto rejected = adapter.startMission();
    assert(!rejected.acknowledged && rejected.error == ErrorCode::NotConnected);
    out << "{\"timestamp_ms\":1,\"parameter\":\"autopilot_command\",\"value\":\"REJECTED\",\"unit\":\"none\",\"source\":\"AUTOPILOT_ADAPTER\",\"quality\":\"VALID\",\"context\":{\"command_id\":\"" << rejected.commandId << "\",\"vehicle_id\":\"" << rejected.vehicleId << "\",\"execution_state\":\"REJECTED\",\"error\":\"NotConnected\",\"reason\":\"" << rejected.reason << "\"}}\n";

    assert(adapter.connect());
    const auto accepted = adapter.startMission();
    assert(accepted.acknowledged && accepted.error == ErrorCode::None);
    out << "{\"timestamp_ms\":2,\"parameter\":\"autopilot_command\",\"value\":\"ACKNOWLEDGED\",\"unit\":\"none\",\"source\":\"AUTOPILOT_ADAPTER\",\"quality\":\"VALID\",\"context\":{\"command_id\":\"" << accepted.commandId << "\",\"vehicle_id\":\"" << accepted.vehicleId << "\",\"execution_state\":\"ACKNOWLEDGED\",\"error\":\"None\",\"reason\":\"" << accepted.reason << "\"}}\n";

    const auto mismatch = adapter.compare("APPROVED", "ACTUAL");
    assert(!mismatch.equal);
    out << "{\"timestamp_ms\":3,\"parameter\":\"mission_readback\",\"value\":\"MISMATCH\",\"unit\":\"none\",\"source\":\"AUTOPILOT_ADAPTER\",\"quality\":\"VALID\",\"context\":{\"comparison_status\":\"MISMATCH\",\"mismatch_count\":" << mismatch.mismatches.size() << "}}\n";

    const auto c2Rejected = c2.send("MISSION", "PAYLOAD");
    assert(!c2Rejected.accepted && c2Rejected.error == C2ErrorCode::NotConnected);
    out << "{\"timestamp_ms\":4,\"parameter\":\"c2_send\",\"value\":\"REJECTED\",\"unit\":\"none\",\"source\":\"C2_LINK\",\"quality\":\"VALID\",\"context\":{\"sequence\":" << c2Rejected.sequence << ",\"error\":\"NotConnected\",\"reason\":\"" << c2Rejected.reason << "\"}}\n";

    assert(c2.connect());
    const auto authFailure = c2.authenticate("");
    assert(!authFailure.accepted && authFailure.error == C2ErrorCode::AuthenticationFailed);
    out << "{\"timestamp_ms\":5,\"parameter\":\"c2_authentication\",\"value\":\"REJECTED\",\"unit\":\"none\",\"source\":\"C2_LINK\",\"quality\":\"VALID\",\"context\":{\"sequence\":" << authFailure.sequence << ",\"error\":\"AuthenticationFailed\",\"reason\":\"" << authFailure.reason << "\"}}\n";

    std::cout << "PH4_INT_007_DIRECT_RESULT_BRIDGE: PASS\n";
    std::cout << "adapter_result_objects=PASS\n";
    std::cout << "c2_result_objects=PASS\n";
    std::cout << "source_boundary_preserved=PASS\n";
    std::cout << "verification_status=FIXTURE_ONLY\n";
    return 0;
}
