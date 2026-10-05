#include "../../OPERATIONS/core/operational_failure_evidence_binding.hpp"
#include "../../PLANNING/mission_transfer_runtime.hpp"
#include "autopilot_adapter_baseline.hpp"
#include "c2_interface_baseline.hpp"

#include <cassert>
#include <string>

namespace {
class ReadbackMismatchAdapter final : public bluesky::autopilot::AutopilotAdapterBaseline {
public:
    ReadbackMismatchAdapter()
        : AutopilotAdapterBaseline(bluesky::autopilot::Identity{
            "VEH-PH4-007", "UAS", "PX4", "1.0", "MAVLink2", "2"}) {}
    std::optional<std::string> readBackMission() override {
        return std::string{"MISMATCHED-READBACK"};
    }
};

bluesky::planning::Mission makeMission() {
    bluesky::planning::Mission mission;
    mission.identity.mission_id = "MIS-PH4-007";
    mission.identity.mission_version = "1";
    mission.vehicle_assignments.push_back({"VEH-PH4-007", "CFG-1"});
    return mission;
}

bluesky::planning::VehicleEquipmentCapabilityProfile makeCapabilities() {
    bluesky::planning::VehicleEquipmentCapabilityProfile profile;
    profile.identity.vehicle_id = "VEH-PH4-007";
    profile.identity.autopilot_type = "PX4";
    profile.identity.supported_protocols = {"MAVLink2"};
    profile.configuration.configuration_version = "CFG-1";
    profile.configuration.valid_for_mission = true;
    return profile;
}

bluesky::operations::DecisionEvidence makeDecision(const std::string& action, const std::string& transition) {
    bluesky::operations::DecisionEvidence decision;
    decision.decision_id = "decision-ph4-007";
    decision.selected_action_id = action;
    decision.state_transition = transition;
    return decision;
}
}

int main() {
    using namespace bluesky::planning;
    ReadbackMismatchAdapter adapter;
    const auto mission_result = MissionTransferRuntime::execute(makeMission(), makeCapabilities(), adapter);
    assert(mission_result.state == MissionTransferState::Failed);
    assert(mission_result.reason == "MISSION_SEMANTIC_VERIFICATION_FAILED");

    const auto mission_evidence = bluesky::operations::bind_failure_evidence(
        makeDecision("mission-transfer", "SELECTED->FAILED"), "FAILED",
        mission_result.reason, "AUTOPILOT_ADAPTER", mission_result.mission_id);
    assert(mission_evidence.decision_id == "decision-ph4-007");
    assert(mission_evidence.action_id == "mission-transfer");
    assert(mission_evidence.state_transition == "SELECTED->FAILED");
    assert(mission_evidence.result == "FAILED");
    assert(mission_evidence.reason == mission_result.reason);
    assert(mission_evidence.source_interface == "AUTOPILOT_ADAPTER");
    assert(mission_evidence.target_object == "MIS-PH4-007");
    assert(mission_evidence.evidence_domain == "EC-20");

    bluesky::c2::C2InterfaceBaseline c2({"C2-PH4-007", "UDP", "TEST", "1"});
    const auto auth_result = c2.authenticate("");
    assert(!auth_result.accepted);
    assert(auth_result.reason == "CREDENTIAL_REFERENCE_REQUIRED");
    const auto auth_evidence = bluesky::operations::bind_failure_evidence(
        makeDecision("c2-authentication", "SELECTED->FAILED"), "REJECTED",
        auth_result.reason, "C2_INTERFACE", c2.getIdentity().endpointId);
    assert(auth_evidence.action_id == "c2-authentication");
    assert(auth_evidence.result == "REJECTED");
    assert(auth_evidence.reason == auth_result.reason);
    assert(auth_evidence.source_interface == "C2_INTERFACE");
    assert(auth_evidence.target_object == "C2-PH4-007");
    assert(auth_evidence.evidence_domain == "EC-20");

    c2.connect();
    const auto frame_result = c2.send("", "");
    assert(!frame_result.accepted);
    assert(frame_result.reason == "CHANNEL_AND_PAYLOAD_REQUIRED");
    const auto frame_evidence = bluesky::operations::bind_failure_evidence(
        makeDecision("c2-dispatch", "SELECTED->FAILED"), "REJECTED",
        frame_result.reason, "C2_INTERFACE", c2.getIdentity().endpointId);
    assert(frame_evidence.action_id == "c2-dispatch");
    assert(frame_evidence.result == "REJECTED");
    assert(frame_evidence.reason == frame_result.reason);
    assert(frame_evidence.target_object == "C2-PH4-007");
    assert(frame_evidence.evidence_domain == "EC-20");
    return 0;
}