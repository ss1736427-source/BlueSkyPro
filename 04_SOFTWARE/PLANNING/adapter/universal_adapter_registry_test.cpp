#include "universal_adapter_registry.hpp"

#include <cassert>
#include <memory>
#include <utility>

using namespace bluesky::planning::adapter;

namespace {

class TestAdapter final : public UniversalAdapter {
public:
    explicit TestAdapter(std::string id, std::string vehicle, std::string equipment)
        : id_(std::move(id)), vehicle_(std::move(vehicle)), equipment_(std::move(equipment)) {}

    AdapterMetadata metadata() const override {
        return {id_, "1.0.0", "TEST", "MOCK-1", "1", "1", {vehicle_}, {equipment_}, {"CAPABILITY-A"}, "1"};
    }

    std::vector<std::string> discover() override { return {}; }
    std::string identify() override { return vehicle_; }
    ResponseEnvelope readVehicleProfile() override { return {}; }
    ResponseEnvelope readConfiguration() override { return {}; }
    ResponseEnvelope readCapabilities() override { return {}; }
    ResponseEnvelope readState() override { return {}; }
    ResponseEnvelope readHealth() override { return {}; }
    ResponseEnvelope readC2State() override { return {}; }
    ResponseEnvelope subscribeTelemetry() override { return {}; }
    ResponseEnvelope readEquipment() override { return {}; }
    ExternalCommand translateCommand(const CanonicalCommand& command) override {
        return {command.request_id, command.object_id, command.operation};
    }
    Error mapError(ErrorCode code, std::string message) const override { return {code, std::move(message)}; }
    std::string identifyEquipment() override { return equipment_; }
    ResponseEnvelope readEquipmentProfile() override { return {}; }
    ResponseEnvelope readEquipmentConfiguration() override { return {}; }
    ResponseEnvelope readEquipmentCapabilities() override { return {}; }
    ResponseEnvelope readEquipmentState() override { return {}; }
    ResponseEnvelope readEquipmentTelemetry() override { return {}; }
    ExternalCommand translateAction(const CanonicalCommand& command) override {
        return {command.request_id, command.object_id, command.operation};
    }
    ResponseEnvelope readDataOutputs() override { return {}; }

private:
    std::string id_;
    std::string vehicle_;
    std::string equipment_;
};

} // namespace

int main() {
    UniversalAdapterRegistry registry;

    assert(registry.adapterIds().empty());
    assert(!registry.registerAdapter(nullptr));
    assert(!registry.unregisterAdapter(""));
    assert(registry.findById("missing") == nullptr);
    assert(registry.findForCapability("CAPABILITY-A") == nullptr);

    assert(registry.registerAdapter(
        std::make_unique<TestAdapter>("ADAPTER-A", "VEHICLE-A", "EQUIPMENT-A")));
    assert(!registry.registerAdapter(
        std::make_unique<TestAdapter>("ADAPTER-A", "VEHICLE-B", "EQUIPMENT-B")));

    assert(registry.findById("ADAPTER-A") != nullptr);
    assert(registry.findForVehicleProfile("VEHICLE-A") == registry.findById("ADAPTER-A"));
    assert(registry.findForEquipmentProfile("EQUIPMENT-A") == registry.findById("ADAPTER-A"));
    assert(registry.findForCapability("CAPABILITY-A") == registry.findById("ADAPTER-A"));
    assert(registry.findForVehicleProfile("VEHICLE-X") == nullptr);
    assert(registry.findForEquipmentProfile("EQUIPMENT-X") == nullptr);
    assert(registry.findForCapability("CAPABILITY-X") == nullptr);
    assert(registry.findForCapability("") == nullptr);

    const AdapterResolutionRequest resolved_request{
        "VEHICLE-A", "EQUIPMENT-A", "MOCK-1", "1", "1", "CAPABILITY-A"};
    const auto resolved = registry.resolve(resolved_request);
    assert(resolved.status == ResolutionStatus::Resolved);
    assert(resolved.adapter == registry.findById("ADAPTER-A"));

    const AdapterResolutionRequest not_found_request{
        "VEHICLE-X", "", "MOCK-1", "1", "1", ""};
    assert(registry.resolve(not_found_request).status == ResolutionStatus::NotFound);

    const AdapterResolutionRequest incompatible_request{
        "VEHICLE-A", "", "OTHER-PROTOCOL", "1", "1", ""};
    assert(registry.resolve(incompatible_request).status == ResolutionStatus::Incompatible);

    const AdapterResolutionRequest capability_request{
        "VEHICLE-A", "", "MOCK-1", "1", "1", "CAPABILITY-B"};
    assert(registry.resolve(capability_request).status == ResolutionStatus::CapabilityUnsupported);

    assert(registry.registerAdapter(
        std::make_unique<TestAdapter>("ADAPTER-B", "VEHICLE-A", "EQUIPMENT-B")));
    assert(registry.resolve(resolved_request).status == ResolutionStatus::Ambiguous);
    assert(registry.findForVehicleProfile("VEHICLE-A") == nullptr);
    assert(registry.findForEquipmentProfile("EQUIPMENT-B") == registry.findById("ADAPTER-B"));
    assert(registry.findForCapability("CAPABILITY-A") == nullptr);

    const auto ids = registry.adapterIds();
    assert(ids.size() == 2);
    assert(ids.front() == "ADAPTER-A");
    assert(ids.back() == "ADAPTER-B");

    assert(registry.unregisterAdapter("ADAPTER-A"));
    assert(registry.findById("ADAPTER-A") == nullptr);
    assert(registry.findForCapability("CAPABILITY-A") == registry.findById("ADAPTER-B"));
    assert(registry.adapterIds().size() == 1);
    assert(!registry.unregisterAdapter("ADAPTER-A"));

    return 0;
}
