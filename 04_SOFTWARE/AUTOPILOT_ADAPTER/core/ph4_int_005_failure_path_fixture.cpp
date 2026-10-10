#include "../../PLANNING/mission_transfer_runtime.hpp"
#include "../core/autopilot_adapter_baseline.hpp"
#include "c2_interface_baseline.hpp"

#include <cassert>
#include <string>

namespace {

class ReadbackMismatchAdapter final : public bluesky::autopilot::AutopilotAdapterBaseline {
public:
    ReadbackMismatchAdapter()
        : AutopilotAdapterBaseline(bluesky::autopilot::Identity{
            "VEH-PH4-005",
            "UAS",
            "PX4",
            "1.0",
            "MAVLink2",
            "2"
        }) {}

    std::optional<std::string> readBackMission() override {
        return std::string{"MISMATCHED-READBACK"};
    }
};

bluesky::planning::Mission makeMission() {
    bluesky::planning::Mission mission;
    mission.identity.mission_id = "MIS-PH4-005";
    mission.identity.mission_version = "1";
    mission.vehicle_assignments.push_back({
        "VEH-PH4-005",
        "CFG-1"
    });
    return mission;
}

bluesky::planning::VehicleEquipmentCapabilityProfile makeCapabilities() {
    bluesky::planning::VehicleEquipmentCapabilityProfile profile;
    profile.identity.vehicle_id = "VEH-PH4-005";
    profile.identity.autopilot_type = "PX4";
    profile.identity.supported_protocols = {"MAVLink2"};
    profile.configuration.configuration_version = "CFG-1";
    profile.configuration.valid_for_mission = true;
    return profile;
}

} // namespace

int main() {
    using namespace bluesky::planning;

    auto mission = makeMission();
    auto capabilities = makeCapabilities();

    ReadbackMismatchAdapter adapter;
    const auto result = MissionTransferRuntime::execute(mission, capabilities, adapter);

    assert(result.state == MissionTransferState::Failed);
    assert(result.reason == "MISSION_SEMANTIC_VERIFICATION_FAILED");
    assert(!result.compiled_mission.empty());
    assert(result.readback_mission == "MISMATCHED-READBACK");

    bluesky::c2::C2InterfaceBaseline c2({
        "C2-PH4-005",
        "UDP",
        "TEST",
        "1"
    });

    const auto auth = c2.authenticate("");
    assert(!auth.accepted);
    assert(auth.error == bluesky::c2::C2ErrorCode::AuthenticationFailed);
    assert(auth.reason == "CREDENTIAL_REFERENCE_REQUIRED");

    c2.connect();
    const auto send = c2.send("", "");
    assert(!send.accepted);
    assert(send.error == bluesky::c2::C2ErrorCode::InvalidFrame);
    assert(send.reason == "CHANNEL_AND_PAYLOAD_REQUIRED");

    return 0;
}
