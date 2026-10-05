#include "autopilot_adapter_baseline.hpp"
#include "c2_interface_baseline.hpp"
#include "ph4_int_009_flight_record_producer.hpp"
#include <cassert>
#include <fstream>
#include <iostream>
#include <string>

int main(int argc, char** argv) {
    const std::string output = argc > 1 ? argv[1] : "ph4_int_010_results.jsonl";
    std::ofstream out(output);
    assert(out.good());

    using namespace bluesky::autopilot;
    using namespace bluesky::c2;
    using namespace bluesky::flight_record;

    FlightRecordProducer producer("FLIGHT-RECORD-PH4-INT-010");
    const auto record = producer.begin();
    const auto timestamp = producer.timestampMs();
    assert(!record.flightRecordId.empty());
    assert(timestamp > 0);

    AutopilotAdapterBaseline adapter(Identity{"FIXTURE-UAV-010", "FIXTURE", "FIXTURE-AUTOPILOT", "1.0", "FIXTURE", "1.0"});
    C2InterfaceBaseline c2(C2Identity{"FIXTURE-UAV-010", "FIXTURE-TRANSPORT", "FIXTURE-PROTOCOL", "1.0"});

    const auto command = adapter.startMission();
    assert(!command.acknowledged && command.error == ErrorCode::NotConnected);
    out << "{\"timestamp_ms\":" << timestamp
        << ",\"parameter\":\"autopilot_command\",\"value\":\"REJECTED\",\"unit\":\"none\",\"source\":\"AUTOPILOT_ADAPTER\",\"quality\":\"VALID\",\"context\":{\"flight_record_id\":\"" << record.flightRecordId
        << "\",\"evidence_domain_id\":\"EC-01\",\"command_id\":\"" << command.commandId << "\",\"vehicle_id\":\"" << command.vehicleId
        << "\",\"execution_state\":\"REJECTED\",\"error\":\"NotConnected\",\"reason\":\"" << command.reason << "\"}}\n";

    assert(c2.connect());
    const auto frame = c2.send("MISSION", "PAYLOAD");
    assert(frame.accepted && frame.error == C2ErrorCode::None);
    out << "{\"timestamp_ms\":" << timestamp
        << ",\"parameter\":\"c2_send\",\"value\":\"ACCEPTED\",\"unit\":\"none\",\"source\":\"C2_LINK\",\"quality\":\"VALID\",\"context\":{\"flight_record_id\":\"" << record.flightRecordId
        << "\",\"evidence_domain_id\":\"EC-17\",\"sequence\":" << frame.sequence << ",\"error\":\"None\",\"reason\":\"" << frame.reason << "\"}}\n";

    std::cout << "PH4_INT_010_OPERATIONAL_EVIDENCE_BRIDGE: PASS\n";
    std::cout << "flight_record_id_propagation=PASS\n";
    std::cout << "producer_timestamp_propagation=PASS\n";
    std::cout << "adapter_c2_results=PASS\n";
    std::cout << "verification_status=FIXTURE_ONLY\n";
    return 0;
}
