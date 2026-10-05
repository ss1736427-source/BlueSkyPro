#include "universal_adapter.hpp"

#include <cassert>
#include <type_traits>
#include <utility>

using namespace bluesky::planning::adapter;

namespace {

class MockAdapter final : public UniversalAdapter {
public:
    AdapterMetadata metadata() const override {
        AdapterMetadata result;
        result.adapter_id = "TEST-ADAPTER-001";
        result.adapter_version = "1.0.0";
        result.vendor = "TEST";
        result.protocol = "MOCK-C2-EQUIPMENT-1";
        result.contract_version = "1";
        result.schema_version = "1";
        result.configuration_schema_version = "1";
        return result;
    }

    std::vector<std::string> discover() override { return {"VEH-TEST-001"}; }
    std::string identify() override { return "VEH-TEST-001"; }
    ResponseEnvelope readVehicleProfile() override { return envelope("VEH-TEST-001"); }
    ResponseEnvelope readConfiguration() override { return envelope("VEH-TEST-001"); }
    ResponseEnvelope readCapabilities() override { return envelope("VEH-TEST-001"); }
    ResponseEnvelope readState() override { return envelope("VEH-TEST-001"); }
    ResponseEnvelope readHealth() override { return envelope("VEH-TEST-001"); }
    ResponseEnvelope readC2State() override { return envelope("VEH-TEST-001"); }
    ResponseEnvelope subscribeTelemetry() override { return envelope("VEH-TEST-001"); }
    ResponseEnvelope readEquipment() override { return envelope("EQ-TEST-001"); }

    ExternalCommand translateCommand(const CanonicalCommand& command) override {
        return {command.request_id, command.object_id, command.operation};
    }

    Error mapError(ErrorCode code, std::string message) const override {
        return {code, std::move(message)};
    }

    std::string identifyEquipment() override { return "EQ-TEST-001"; }
    ResponseEnvelope readEquipmentProfile() override { return envelope("EQ-TEST-001"); }
    ResponseEnvelope readEquipmentConfiguration() override { return envelope("EQ-TEST-001"); }
    ResponseEnvelope readEquipmentCapabilities() override { return envelope("EQ-TEST-001"); }
    ResponseEnvelope readEquipmentState() override { return envelope("EQ-TEST-001"); }
    ResponseEnvelope readEquipmentTelemetry() override { return envelope("EQ-TEST-001"); }

    ExternalCommand translateAction(const CanonicalCommand& command) override {
        return {command.request_id, command.object_id, command.operation};
    }

    ResponseEnvelope readDataOutputs() override { return envelope("EQ-TEST-001"); }

private:
    static ResponseEnvelope envelope(const std::string& object_id) {
        return {"REQ-001", "TEST-ADAPTER-001", "1.0.0", object_id, "1", "2026-01-01T00:00:00Z", "valid", "nominal", "mock"};
    }
};

} // namespace

int main() {
    static_assert(std::is_polymorphic_v<UniversalAdapter>);
    static_assert(std::has_virtual_destructor_v<UniversalAdapter>);

    MockAdapter adapter;
    const auto meta = adapter.metadata();
    assert(meta.adapter_id == "TEST-ADAPTER-001");
    assert(meta.adapter_version == "1.0.0");
    assert(meta.contract_version == "1");
    assert(meta.schema_version == "1");
    assert(adapter.identify() == "VEH-TEST-001");
    assert(adapter.identifyEquipment() == "EQ-TEST-001");

    const auto vehicle = adapter.readVehicleProfile();
    const auto equipment = adapter.readEquipment();
    assert(vehicle.object_id == "VEH-TEST-001");
    assert(equipment.object_id == "EQ-TEST-001");
    assert(vehicle.validity == "valid");
    assert(vehicle.quality == "nominal");

    const CanonicalCommand command{"REQ-002", "EQ-TEST-001", "CAPTURE"};
    const auto translated = adapter.translateAction(command);
    assert(translated.request_id == command.request_id);
    assert(translated.object_id == command.object_id);
    assert(translated.operation == "CAPTURE");

    const auto error = adapter.mapError(ErrorCode::StaleData, "stale telemetry");
    assert(error.code == ErrorCode::StaleData);
    assert(error.message == "stale telemetry");

    return 0;
}
