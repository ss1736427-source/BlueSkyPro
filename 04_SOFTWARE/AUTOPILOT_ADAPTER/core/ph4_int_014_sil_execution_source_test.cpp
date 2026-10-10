#include "autopilot_adapter_baseline.hpp"
#include "ph4_int_014_sil_execution_source.hpp"

#include <cassert>
#include <iostream>

int main() {
    using namespace bluesky::autopilot;

    AutopilotAdapterBaseline adapter({"UAV-SIL-014", "MULTIROTOR", "SIL-BASELINE", "SIL", "MAVLink2", "2"});
    SilExecutionSource source("FLIGHT-RECORD-PH4-INT-014");

    assert(adapter.connect());
    const auto result = adapter.startMission();
    assert(result.acknowledged);

    const auto event = source.emit(result.vehicleId, result.commandId, "ACKNOWLEDGED");
    assert(event.timestampMs > 0);
    assert(event.flightRecordId == "FLIGHT-RECORD-PH4-INT-014");
    assert(event.vehicleId == "UAV-SIL-014");
    assert(event.source == "SIL_EXECUTION_SOURCE");

    std::cout << "PH4_INT_014_SIL_EXECUTION_SOURCE: PASS\n";
    std::cout << "universal_adapter_to_sil_source=PASS\n";
    std::cout << "flight_record_binding=PASS\n";
    std::cout << "verification_status=SIL_FIXTURE_ONLY\n";
    return 0;
}
