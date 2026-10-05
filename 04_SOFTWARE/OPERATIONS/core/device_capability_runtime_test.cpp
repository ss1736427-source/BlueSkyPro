#include "device_capability_runtime.hpp"

#include <cassert>

using namespace bluesky::operations;

int main() {
    DeviceCapabilityRuntime runtime;

    DeviceCapabilitySnapshot invalid{
        "",
        "UAV-044",
        "AUTOPILOT-044",
        "FW-1",
        "MAVLink2",
        "2",
        DeviceCapabilityLifecycle::Discovered,
        {"mission.upload"}};
    assert(runtime.registerSnapshot(invalid) == DeviceCapabilityResult::InvalidSnapshot);

    DeviceCapabilitySnapshot discovered{
        "DEVICE-044",
        "UAV-044",
        "AUTOPILOT-044",
        "FW-1",
        "MAVLink2",
        "2",
        DeviceCapabilityLifecycle::Discovered,
        {"command.arm", "mission.upload", "command.arm", "telemetry.position"}};

    assert(runtime.registerSnapshot(discovered) == DeviceCapabilityResult::Registered);
    assert(runtime.registerSnapshot(discovered) == DeviceCapabilityResult::AlreadyRegistered);
    assert(runtime.size() == 1);

    DeviceCapabilitySnapshot resolved{};
    assert(runtime.resolve("DEVICE-044", resolved));
    assert(resolved.capabilities.size() == 3);
    assert(resolved.capabilities[0] == "command.arm");
    assert(resolved.capabilities[1] == "mission.upload");
    assert(resolved.capabilities[2] == "telemetry.position");

    DeviceCapabilitySnapshot changed = resolved;
    changed.capabilities.push_back("mission.readback");
    assert(runtime.registerSnapshot(changed) == DeviceCapabilityResult::InvalidSnapshot);
    assert(runtime.updateSnapshot(changed) == DeviceCapabilityResult::Updated);

    assert(runtime.setLifecycle(
               "DEVICE-044",
               DeviceCapabilityLifecycle::Validated) ==
           DeviceCapabilityResult::Updated);

    assert(runtime.resolve("DEVICE-044", resolved));
    assert(resolved.lifecycle == DeviceCapabilityLifecycle::Validated);
    assert(resolved.capabilities.size() == 4);

    assert(runtime.updateSnapshot(
               DeviceCapabilitySnapshot{
                   "DEVICE-MISSING",
                   "UAV",
                   "AP",
                   "FW",
                   "MAVLink2",
                   "2",
                   DeviceCapabilityLifecycle::Discovered,
                   {"telemetry.position"}}) ==
           DeviceCapabilityResult::NotFound);

    assert(runtime.setLifecycle(
               "DEVICE-MISSING",
               DeviceCapabilityLifecycle::Validated) ==
           DeviceCapabilityResult::NotFound);

    const auto all = runtime.snapshot();
    assert(all.size() == 1);
    assert(all.front().device_id == "DEVICE-044");

    return 0;
}
