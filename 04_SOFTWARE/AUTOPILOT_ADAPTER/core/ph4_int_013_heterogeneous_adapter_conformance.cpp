#include "autopilot_adapter_baseline.hpp"

#include <cassert>
#include <fstream>
#include <iostream>
#include <string>

namespace {
void writeResult(std::ofstream& out, const bluesky::autopilot::CommandResult& result,
                 const std::string& flightRecordId, const std::string& vehicleType,
                 const std::string& protocol) {
    out << "{\"timestamp_ms\":1,\"parameter\":\"adapter_conformance\",\"value\":\""
        << (result.acknowledged ? "ACKNOWLEDGED" : "REJECTED")
        << "\",\"unit\":\"none\",\"source\":\"BLUESKY_EVENT\",\"quality\":\"VALID\",\"context\":{\"flight_record_id\":\""
        << flightRecordId << "\",\"evidence_domain_id\":\"EC-01\",\"vehicle_id\":\""
        << result.vehicleId << "\",\"vehicle_type\":\"" << vehicleType
        << "\",\"protocol\":\"" << protocol << "\",\"command_id\":\""
        << result.commandId << "\",\"execution_state\":\"ACKNOWLEDGED\"}}\n";
}
}

int main(int argc, char** argv) {
    const std::string output = argc > 1 ? argv[1] : "ph4_int_013_results.jsonl";
    std::ofstream out(output);
    assert(out.good());

    using namespace bluesky::autopilot;
    AutopilotAdapterBaseline multirotor({"UAV-MR-013", "MULTIROTOR", "BASELINE-MR", "FIXTURE", "MAVLink2", "2"});
    AutopilotAdapterBaseline fixedWing({"UAV-FW-013", "FIXED_WING", "BASELINE-FW", "FIXTURE", "MAVLink2", "2"});

    assert(multirotor.getVehicleIdentity().vehicleType == "MULTIROTOR");
    assert(fixedWing.getVehicleIdentity().vehicleType == "FIXED_WING");
    assert(multirotor.getProtocolVersion() == "2");
    assert(fixedWing.getProtocolVersion() == "2");

    assert(multirotor.connect());
    assert(fixedWing.connect());
    const auto mrResult = multirotor.startMission();
    const auto fwResult = fixedWing.startMission();
    assert(mrResult.acknowledged);
    assert(fwResult.acknowledged);

    writeResult(out, mrResult, "FLIGHT-RECORD-PH4-INT-013-MR", "MULTIROTOR", "MAVLink2");
    writeResult(out, fwResult, "FLIGHT-RECORD-PH4-INT-013-FW", "FIXED_WING", "MAVLink2");

    std::cout << "PH4_INT_013_HETEROGENEOUS_ADAPTER_CONFORMANCE: PASS\n";
    std::cout << "multirotor_universal_contract=PASS\n";
    std::cout << "fixed_wing_universal_contract=PASS\n";
    std::cout << "verification_status=FIXTURE_ONLY\n";
    return 0;
}
