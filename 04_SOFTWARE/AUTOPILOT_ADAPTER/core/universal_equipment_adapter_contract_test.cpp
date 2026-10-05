#include "universal_equipment_adapter.hpp"

#include <cassert>
#include <type_traits>

namespace {

class ContractEquipmentAdapter final : public bluesky::equipment::UniversalEquipmentAdapter {
public:
    bluesky::equipment::Identity identify() const override {
        return {"EQ-TEST-001", "EQ-PROFILE-001", "TEST", "CAMERA-TEST", "SN-001"};
    }

    std::string readProfile() const override { return "EQ-PROFILE-001"; }

    std::string readConfiguration() const override { return "EQ-CONFIG-001"; }

    std::vector<bluesky::equipment::Capability> readCapabilities() const override {
        return {
            {"CAPTURE", bluesky::equipment::AvailabilityState::Supported},
            {"START_RECORDING", bluesky::equipment::AvailabilityState::NotVerified},
        };
    }

    bluesky::equipment::EquipmentState readState() const override {
        return bluesky::equipment::EquipmentState::Ready;
    }

    std::optional<bluesky::equipment::Telemetry> readTelemetry() const override {
        return bluesky::equipment::Telemetry{
            "EQ-TEST-001", "temperature=42", 1000, 1010, true, false, "GOOD"};
    }

    bluesky::equipment::ActionResult translateAction(const std::string& action) override {
        if (action == "CAPTURE") {
            return {"ACT-001", "EQ-TEST-001", true,
                    bluesky::equipment::ErrorCode::None, "", 1000, 1010};
        }

        return {"ACT-002", "EQ-TEST-001", false,
                bluesky::equipment::ErrorCode::Unsupported, "unsupported action", 1000, 1010};
    }

    std::vector<bluesky::equipment::DataOutput> readDataOutputs() const override {
        return {{"OUT-001", "IMAGE", "JPEG"}};
    }
};

} // namespace

int main() {
    static_assert(std::is_abstract_v<bluesky::equipment::UniversalEquipmentAdapter>);
    ContractEquipmentAdapter adapter;

    const auto identity = adapter.identify();
    assert(identity.equipmentId == "EQ-TEST-001");
    assert(identity.profileId == "EQ-PROFILE-001");
    assert(identity.serialNumber == "SN-001");

    assert(adapter.readProfile() == "EQ-PROFILE-001");
    assert(adapter.readConfiguration() == "EQ-CONFIG-001");

    const auto capabilities = adapter.readCapabilities();
    assert(capabilities.size() == 2);
    assert(capabilities[0].id == "CAPTURE");
    assert(capabilities[0].availability == bluesky::equipment::AvailabilityState::Supported);
    assert(capabilities[1].availability == bluesky::equipment::AvailabilityState::NotVerified);

    assert(adapter.readState() == bluesky::equipment::EquipmentState::Ready);

    const auto telemetry = adapter.readTelemetry();
    assert(telemetry.has_value());
    assert(telemetry->equipmentId == "EQ-TEST-001");
    assert(telemetry->valid);
    assert(!telemetry->stale);
    assert(telemetry->sourceTimestampMs == 1000);
    assert(telemetry->adapterTimestampMs == 1010);
    assert(telemetry->quality == "GOOD");

    const auto capture = adapter.translateAction("CAPTURE");
    assert(capture.accepted);
    assert(capture.error == bluesky::equipment::ErrorCode::None);
    assert(capture.equipmentId == "EQ-TEST-001");

    const auto unsupported = adapter.translateAction("UNKNOWN");
    assert(!unsupported.accepted);
    assert(unsupported.error == bluesky::equipment::ErrorCode::Unsupported);

    const auto outputs = adapter.readDataOutputs();
    assert(outputs.size() == 1);
    assert(outputs.front().outputId == "OUT-001");
    assert(outputs.front().type == "IMAGE");
    assert(outputs.front().format == "JPEG");

    return 0;
}
