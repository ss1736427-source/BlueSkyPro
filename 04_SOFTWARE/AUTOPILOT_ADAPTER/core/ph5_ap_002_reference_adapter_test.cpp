#include "px4_mavlink2_reference_adapter.hpp"
#include <cassert>

int main() {
    bluesky::autopilot::Identity identity;
    identity.vehicleId = "PX4-REFERENCE";
    identity.vehicleType = "MULTIROTOR";
    identity.firmwareVersion = "TEST";

    bluesky::autopilot::Px4Mavlink2ReferenceAdapter adapter(identity);

    const auto resolved = adapter.getVehicleIdentity();
    assert(resolved.vehicleId == "PX4-REFERENCE");
    assert(resolved.autopilot == "PX4");
    assert(resolved.protocol == "MAVLINK2");
    assert(adapter.getAutopilotIdentity() == "PX4");
    assert(adapter.getProtocolVersion() == "MAVLINK2");

    const auto capabilities = adapter.getCapabilities();
    assert(!capabilities.commands.empty());
    assert(!capabilities.missionFeatures.empty());
    assert(!capabilities.equipmentFeatures.empty());

    const auto manifest = adapter.getCapabilityManifest();
    assert(manifest.fcsName == "PX4");
    assert(manifest.protocol == "MAVLINK2");

    const auto compatible = adapter.checkCompatibility({"MAVLINK2", "MAVLINK2", "MISSION_UPLOAD"});
    assert(compatible.compatible);

    const auto rejected = adapter.checkCompatibility({"MAVLINK2", "MAVLINK2", "UNSUPPORTED_FEATURE"});
    assert(!rejected.compatible);
    assert(rejected.error == bluesky::autopilot::ErrorCode::CapabilityMissing);

    return 0;
}
