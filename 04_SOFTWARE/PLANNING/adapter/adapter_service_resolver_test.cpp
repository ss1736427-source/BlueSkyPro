#include "adapter_service_resolver.hpp"

#include <cassert>
#include <memory>
#include <utility>

using namespace bluesky::planning::adapter;

namespace {

class TestAdapter final : public UniversalAdapter {
public:
    AdapterMetadata metadata() const override {
        return {"SERVICE-ADAPTER-001", "1.0.0", "TEST", "MOCK-1", "1", "1",
                {"VEHICLE-SERVICE"}, {"EQUIPMENT-SERVICE"}, {"CAPTURE"}, "1"};
    }

    std::vector<std::string> discover() override { return {}; }
    std::string identify() override { return "VEHICLE-SERVICE"; }
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
    std::string identifyEquipment() override { return "EQUIPMENT-SERVICE"; }
    ResponseEnvelope readEquipmentProfile() override { return {}; }
    ResponseEnvelope readEquipmentConfiguration() override { return {}; }
    ResponseEnvelope readEquipmentCapabilities() override { return {}; }
    ResponseEnvelope readEquipmentState() override { return {}; }
    ResponseEnvelope readEquipmentTelemetry() override { return {}; }
    ExternalCommand translateAction(const CanonicalCommand& command) override {
        return {command.request_id, command.object_id, command.operation};
    }
    ResponseEnvelope readDataOutputs() override { return {}; }
};

} // namespace

int main() {
    UniversalAdapterRegistry registry;
    assert(registry.registerAdapter(std::make_unique<TestAdapter>()));

    AdapterServiceResolver resolver(registry);

    const AdapterResolutionRequest request{
        "VEHICLE-SERVICE", "EQUIPMENT-SERVICE", "MOCK-1", "1", "1", "CAPTURE"};
    const auto result = resolver.resolve(request);

    assert(result.status == ResolutionStatus::Resolved);
    assert(result.adapter == registry.findById("SERVICE-ADAPTER-001"));

    const AdapterResolutionRequest missing{"VEHICLE-MISSING", "", "MOCK-1", "1", "1", ""};
    assert(resolver.resolve(missing).status == ResolutionStatus::NotFound);

    return 0;
}
