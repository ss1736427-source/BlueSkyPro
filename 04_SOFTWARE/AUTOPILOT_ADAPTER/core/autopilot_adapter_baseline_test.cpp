#include "autopilot_adapter_baseline.hpp"

#include <cassert>
#include <iostream>

int main() {
    using namespace bluesky::autopilot;

    Identity id;
    id.vehicleId = "UAV-001";
    id.vehicleType = "FIXED_WING";
    id.autopilot = "BASELINE";
    id.firmwareVersion = "FW-TEST-1";
    id.protocol = "ABSTRACT";
    id.protocolVersion = "1";

    AutopilotAdapterBaseline adapter(id);

    assert(adapter.getVehicleIdentity().vehicleId == "UAV-001");
    assert(adapter.getConnectionState() == ConnectionState::Disconnected);

    auto rejected = adapter.arm();
    assert(rejected.executionState == ExecutionState::Rejected);
    assert(rejected.error == ErrorCode::NotConnected);

    assert(adapter.connect());
    assert(adapter.getConnectionState() == ConnectionState::Connected);
    assert(adapter.getLinkMetrics().healthy);

    auto armed = adapter.arm();
    assert(armed.acknowledged);
    assert(armed.executionState == ExecutionState::Acknowledged);
    assert(armed.vehicleId == "UAV-001");

    auto writeResult = adapter.write("CFG-001");
    assert(writeResult.acknowledged);
    assert(adapter.verify("CFG-001").executionState == ExecutionState::Completed);

    assert(adapter.uploadMission("MIS-001:UAV-001"));
    assert(adapter.downloadMission().value() == "MIS-001:UAV-001");
    assert(adapter.readBackMission().value() == "MIS-001:UAV-001");
    assert(adapter.compareMission("MIS-001:UAV-001", "MIS-001:UAV-001").equal);
    assert(!adapter.compareMission("MIS-001:UAV-001", "MIS-002:UAV-001").equal);

    adapter.disconnect();
    assert(adapter.getConnectionState() == ConnectionState::Disconnected);

    std::cout << "PASS: EC-02 normalized autopilot adapter baseline\n";
    return 0;
}
