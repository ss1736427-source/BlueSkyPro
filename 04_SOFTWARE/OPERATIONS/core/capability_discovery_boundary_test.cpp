#include "capability_discovery_boundary.hpp"

#include <cassert>

using namespace bluesky::operations;

int main() {
    DeviceIdentityRuntime identities;
    DeviceCapabilityRuntime capabilities;
    CapabilityDiscoveryBoundary boundary(identities, capabilities);

    CapabilityDiscoveryReport report{
        "DEVICE-045",
        "FW-1",
        "MAVLink2",
        "2",
        {"telemetry.position", "mission.upload", "telemetry.position"}};

    assert(boundary.ingest(report) == CapabilityDiscoveryResult::IdentityNotFound);

    assert(identities.registerIdentity(DeviceIdentity{
               "DEVICE-045",
               "UAV-045",
               "AUTOPILOT-045",
               "COMP-1",
               "fingerprint-045",
               DeviceIdentityLifecycle::Registered}) ==
           DeviceIdentityRegistrationResult::Registered);

    assert(boundary.ingest(report) == CapabilityDiscoveryResult::Registered);

    DeviceCapabilitySnapshot snapshot{};
    assert(capabilities.resolve("DEVICE-045", snapshot));
    assert(snapshot.vehicle_id == "UAV-045");
    assert(snapshot.capabilities.size() == 2);
    assert(snapshot.capabilities[0] == "mission.upload");
    assert(snapshot.capabilities[1] == "telemetry.position");

    assert(boundary.ingest(CapabilityDiscoveryReport{
               "DEVICE-045",
               "FW-2",
               "MAVLink2",
               "2",
               {"mission.upload", "mission.readback"}}) ==
           CapabilityDiscoveryResult::Updated);

    assert(capabilities.resolve("DEVICE-045", snapshot));
    assert(snapshot.firmware == "FW-2");
    assert(snapshot.capabilities.size() == 2);

    assert(identities.setLifecycle(
               "DEVICE-045",
               DeviceIdentityLifecycle::Revoked));

    assert(boundary.ingest(report) == CapabilityDiscoveryResult::IdentityRevoked);

    assert(boundary.ingest(CapabilityDiscoveryReport{
               "",
               "FW",
               "MAVLink2",
               "2",
               {"telemetry.position"}}) ==
           CapabilityDiscoveryResult::InvalidReport);

    return 0;
}
