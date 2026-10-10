#include "capability_admission_boundary.hpp"

#include <cassert>

using namespace bluesky::operations;

namespace {

DeviceCapabilitySnapshot validated()
{
    DeviceCapabilitySnapshot snapshot;
    snapshot.device_id = "dev-047";
    snapshot.vehicle_id = "veh-047";
    snapshot.autopilot_id = "ap-047";
    snapshot.firmware = "fw-1";
    snapshot.protocol = "MAVLink";
    snapshot.protocol_version = "2";
    snapshot.lifecycle = DeviceCapabilityLifecycle::Validated;
    snapshot.capabilities = {"mission.upload", "mission.start", "telemetry"};
    return snapshot;
}

} // namespace

int main()
{
    DeviceCapabilityRuntime runtime;
    assert(runtime.registerSnapshot(validated())
           == DeviceCapabilityResult::Registered);

    CapabilityCompatibilityGate gate(runtime);
    CapabilityAdmissionBoundary admission(gate);

    const auto admitted = admission.admit({
        "dev-047",
        "mission.upload",
        {"mission.upload"}
    });
    assert(admitted.state == CapabilityAdmissionState::Admitted);
    assert(admitted.compatibility == CapabilityCompatibilityResult::Compatible);

    const auto missing = admission.admit({
        "dev-047",
        "mission.start",
        {"mission.abort"}
    });
    assert(missing.state == CapabilityAdmissionState::Rejected);
    assert(missing.compatibility == CapabilityCompatibilityResult::MissingCapability);
    assert(missing.missing_capability == "mission.abort");

    const auto unknown = admission.admit({
        "dev-unknown",
        "mission.upload",
        {"mission.upload"}
    });
    assert(unknown.state == CapabilityAdmissionState::Rejected);
    assert(unknown.compatibility == CapabilityCompatibilityResult::CapabilityNotFound);

    const auto invalid_action = admission.admit({
        "dev-047",
        "",
        {"mission.upload"}
    });
    assert(invalid_action.state == CapabilityAdmissionState::Rejected);
    assert(invalid_action.reason == "ACTION_ID_REQUIRED");

    const auto invalid_requirements = admission.admit({
        "dev-047",
        "mission.upload",
        {}
    });
    assert(invalid_requirements.state == CapabilityAdmissionState::Rejected);
    assert(invalid_requirements.compatibility == CapabilityCompatibilityResult::InvalidRequest);

    DeviceCapabilitySnapshot discovered = validated();
    discovered.lifecycle = DeviceCapabilityLifecycle::Discovered;
    assert(runtime.updateSnapshot(discovered)
           == DeviceCapabilityResult::Updated);

    const auto unvalidated = admission.admit({
        "dev-047",
        "mission.upload",
        {"mission.upload"}
    });
    assert(unvalidated.state == CapabilityAdmissionState::Rejected);
    assert(unvalidated.compatibility == CapabilityCompatibilityResult::CapabilityNotValidated);

    return 0;
}
