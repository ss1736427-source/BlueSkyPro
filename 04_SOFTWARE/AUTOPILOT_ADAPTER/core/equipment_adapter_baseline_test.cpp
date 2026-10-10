#include "equipment_adapter_baseline.hpp"

#include <cassert>

using namespace bluesky::equipment;

int main() {
    EquipmentAdapterBaseline adapter(
        {"EQ-CAM-001", "CAMERA-PROFILE-001", "BlueSky", "CAM-TEST", "SN-001"},
        "CAMERA-PROFILE-001",
        "EQ-CONFIG-001",
        {{"CAPTURE", AvailabilityState::Supported},
         {"START_RECORDING", AvailabilityState::NotVerified}},
        {{"OUT-IMAGE", "IMAGE", "JPEG"}});

    assert(adapter.identify().equipmentId == "EQ-CAM-001");
    assert(adapter.identify().serialNumber == "SN-001");
    assert(adapter.readProfile() == "CAMERA-PROFILE-001");
    assert(adapter.readConfiguration() == "EQ-CONFIG-001");
    assert(adapter.readState() == EquipmentState::Off);

    adapter.setState(EquipmentState::Ready);
    assert(adapter.readState() == EquipmentState::Ready);

    adapter.setTelemetry({"EQ-CAM-001", "temperature=42", 1000, 1010, true, false, "GOOD"});
    const auto telemetry = adapter.readTelemetry();
    assert(telemetry.has_value());
    assert(telemetry->valid);
    assert(!telemetry->stale);
    assert(telemetry->sourceTimestampMs == 1000);
    assert(telemetry->adapterTimestampMs == 1010);

    const auto capture = adapter.translateAction("CAPTURE");
    assert(capture.accepted);
    assert(capture.error == ErrorCode::None);
    assert(capture.equipmentId == "EQ-CAM-001");

    const auto unverified = adapter.translateAction("START_RECORDING");
    assert(!unverified.accepted);
    assert(unverified.error == ErrorCode::CapabilityMissing);

    const auto unknown = adapter.translateAction("UNKNOWN");
    assert(!unknown.accepted);
    assert(unknown.error == ErrorCode::Unsupported);

    const auto outputs = adapter.readDataOutputs();
    assert(outputs.size() == 1);
    assert(outputs.front().type == "IMAGE");
    assert(outputs.front().format == "JPEG");

    return 0;
}
