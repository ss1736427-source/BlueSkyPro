#include "device_identity_runtime.hpp"

#include <cassert>

using namespace bluesky::operations;

static DeviceIdentity identity(const char* device,
                               const char* vehicle,
                               const char* autopilot,
                               const char* component,
                               const char* fingerprint) {
    return {device, vehicle, autopilot, component, fingerprint,
            DeviceIdentityLifecycle::Registered};
}

int main() {
    DeviceIdentityRuntime runtime;

    const auto a = identity("DEV-A", "UAV-A", "AP-A", "FC-A", "CERT-A");
    const auto b = identity("DEV-B", "UAV-B", "AP-B", "FC-B", "CERT-B");

    assert(runtime.registerIdentity(a) ==
           DeviceIdentityRegistrationResult::Registered);
    assert(runtime.registerIdentity(a) ==
           DeviceIdentityRegistrationResult::AlreadyRegistered);
    assert(runtime.registerIdentity(b) ==
           DeviceIdentityRegistrationResult::Registered);

    const auto resolved = runtime.resolveDevice("DEV-A");
    assert(resolved.has_value());
    assert(resolved->vehicle_id == "UAV-A");

    const auto by_vehicle = runtime.resolveVehicle("UAV-B");
    assert(by_vehicle.has_value());
    assert(by_vehicle->device_id == "DEV-B");

    auto collision = a;
    collision.vehicle_id = "UAV-C";
    assert(runtime.registerIdentity(collision) ==
           DeviceIdentityRegistrationResult::IdentityCollision);

    auto credential_collision = b;
    credential_collision.device_id = "DEV-C";
    credential_collision.credential_fingerprint = "CERT-A";
    assert(runtime.registerIdentity(credential_collision) ==
           DeviceIdentityRegistrationResult::CredentialCollision);

    auto invalid = b;
    invalid.device_id.clear();
    assert(runtime.registerIdentity(invalid) ==
           DeviceIdentityRegistrationResult::InvalidIdentity);

    assert(runtime.setLifecycle("DEV-A", DeviceIdentityLifecycle::Active));
    assert(runtime.resolveDevice("DEV-A")->lifecycle ==
           DeviceIdentityLifecycle::Active);

    assert(runtime.setLifecycle("DEV-A", DeviceIdentityLifecycle::Revoked));
    assert(runtime.resolveDevice("DEV-A")->lifecycle ==
           DeviceIdentityLifecycle::Revoked);

    assert(runtime.unregisterIdentity("DEV-A"));
    assert(!runtime.resolveDevice("DEV-A").has_value());
    assert(runtime.snapshot().size() == 1);

    return 0;
}
