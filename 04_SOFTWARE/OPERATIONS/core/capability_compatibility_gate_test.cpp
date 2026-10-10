#include "capability_compatibility_gate.hpp"

#include <cassert>

using namespace bluesky::operations;

namespace {

DeviceCapabilitySnapshot validatedSnapshot()
{
    DeviceCapabilitySnapshot snapshot;
    snapshot.device_id = "dev-046";
    snapshot.vehicle_id = "veh-046";
    snapshot.autopilot_id = "ap-046";
    snapshot.firmware = "fw-1";
    snapshot.protocol = "MAVLink";
    snapshot.protocol_version = "2";
    snapshot.lifecycle = DeviceCapabilityLifecycle::Validated;
    snapshot.capabilities = {"mission.upload", "telemetry", "mission.upload"};
    return snapshot;
}

} // namespace

int main()
{
    DeviceCapabilityRuntime runtime;
    assert(runtime.registerSnapshot(validatedSnapshot())
           == DeviceCapabilityResult::Registered);

    CapabilityCompatibilityGate gate(runtime);

    const auto compatible = gate.evaluate({
        "dev-046",
        {"telemetry", "mission.upload", "telemetry"}
    });
    assert(compatible.result == CapabilityCompatibilityResult::Compatible);
    assert(compatible.missing_capability.empty());

    const auto missing = gate.evaluate({
        "dev-046",
        {"telemetry", "camera.capture"}
    });
    assert(missing.result == CapabilityCompatibilityResult::MissingCapability);
    assert(missing.missing_capability == "camera.capture");

    DeviceCapabilitySnapshot discovered = validatedSnapshot();
    discovered.lifecycle = DeviceCapabilityLifecycle::Discovered;
    assert(runtime.updateSnapshot(discovered)
           == DeviceCapabilityResult::Updated);

    const auto unvalidated = gate.evaluate({
        "dev-046",
        {"telemetry"}
    });
    assert(unvalidated.result == CapabilityCompatibilityResult::CapabilityNotValidated);

    const auto unknown = gate.evaluate({
        "dev-unknown",
        {"telemetry"}
    });
    assert(unknown.result == CapabilityCompatibilityResult::CapabilityNotFound);

    const auto empty_device = gate.evaluate({
        "",
        {"telemetry"}
    });
    assert(empty_device.result == CapabilityCompatibilityResult::InvalidRequest);

    const auto empty_requirement = gate.evaluate({
        "dev-046",
        {}
    });
    assert(empty_requirement.result == CapabilityCompatibilityResult::InvalidRequest);

    const auto empty_capability = gate.evaluate({
        "dev-046",
        {"telemetry", ""}
    });
    assert(empty_capability.result == CapabilityCompatibilityResult::InvalidRequest);

    return 0;
}
