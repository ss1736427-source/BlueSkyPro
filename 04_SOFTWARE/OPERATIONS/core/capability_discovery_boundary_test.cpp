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

    const auto missing_identity_result = boundary.ingest(report);
    assert(missing_identity_result == CapabilityDiscoveryResult::IdentityNotFound);

    const auto identity_registration = identities.registerIdentity(DeviceIdentity{
        "DEVICE-045",
        "UAV-045",
        "AUTOPILOT-045",
        "COMP-1",
        "fingerprint-045",
        DeviceIdentityLifecycle::Registered});
    assert(identity_registration == DeviceIdentityRegistrationResult::Registered);

    const auto initial_discovery_result = boundary.ingest(report);
    assert(initial_discovery_result == CapabilityDiscoveryResult::Registered);

    DeviceCapabilitySnapshot snapshot{};
    const bool initial_snapshot_resolved = capabilities.resolve("DEVICE-045", snapshot);
    assert(initial_snapshot_resolved);
    assert(snapshot.vehicle_id == "UAV-045");
    assert(snapshot.capabilities.size() == 2);
    assert(snapshot.capabilities[0] == "mission.upload");
    assert(snapshot.capabilities[1] == "telemetry.position");

    const auto update_result = boundary.ingest(CapabilityDiscoveryReport{
        "DEVICE-045",
        "FW-2",
        "MAVLink2",
        "2",
        {"mission.upload", "mission.readback"}});
    assert(update_result == CapabilityDiscoveryResult::Updated);

    const bool updated_snapshot_resolved = capabilities.resolve("DEVICE-045", snapshot);
    assert(updated_snapshot_resolved);
    assert(snapshot.firmware == "FW-2");
    assert(snapshot.capabilities.size() == 2);

    const bool identity_revoked = identities.setLifecycle(
        "DEVICE-045", DeviceIdentityLifecycle::Revoked);
    assert(identity_revoked);

    const auto revoked_identity_result = boundary.ingest(report);
    assert(revoked_identity_result == CapabilityDiscoveryResult::IdentityRevoked);

    assert(boundary.ingest(CapabilityDiscoveryReport{
               "",
               "FW",
               "MAVLink2",
               "2",
               {"telemetry.position"}}) ==
           CapabilityDiscoveryResult::InvalidReport);

    return 0;
}
