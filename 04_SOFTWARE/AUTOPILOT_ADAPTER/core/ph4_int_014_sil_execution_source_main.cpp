#include "autopilot_adapter_baseline.hpp"
#include "ph4_int_014_sil_execution_source.hpp"

#include <fstream>
#include <iostream>

int main(int argc, char** argv) {
    using namespace bluesky::autopilot;
    const std::string output = argc > 1 ? argv[1] : "ph4_int_014_events.jsonl";
    std::ofstream out(output);
    if (!out.good()) return 2;

    AutopilotAdapterBaseline adapter({"UAV-SIL-014", "MULTIROTOR", "SIL-BASELINE", "SIL", "MAVLink2", "2"});
    SilExecutionSource source("FLIGHT-RECORD-PH4-INT-014");
    if (!adapter.connect()) return 3;
    const auto result = adapter.startMission();
    if (!result.acknowledged) return 4;
    const auto event = source.emit(result.vehicleId, result.commandId, "ACKNOWLEDGED");

    out << "{\"timestamp_ms\":" << event.timestampMs
        << ",\"parameter\":\"sil_execution_state\",\"value\":\""
        << event.executionState
        << "\",\"unit\":\"none\",\"source\":\"SIL_EXECUTION_SOURCE\",\"quality\":\"VALID\",\"context\":{\"flight_record_id\":\""
        << event.flightRecordId << "\",\"evidence_domain_id\":\"EC-01\",\"vehicle_id\":\""
        << event.vehicleId << "\",\"command_id\":\"" << event.commandId
        << "\",\"execution_state\":\"" << event.executionState
        << "\",\"execution_source\":\"SIL_FIXTURE\"}}\n";

    std::cout << "PH4_INT_014_SIL_EXECUTION_SOURCE: PASS\n";
    std::cout << "verification_status=SIL_FIXTURE_ONLY\n";
    return 0;
}
