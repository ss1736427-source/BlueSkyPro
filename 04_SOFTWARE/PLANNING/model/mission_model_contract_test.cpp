#include "../model/mission_model.hpp"

#include <cassert>
#include <string>

using namespace bluesky::planning;

int main() {
    Mission mission;
    mission.identity.mission_id = "MISSION-001";
    mission.identity.mission_version = "1";
    mission.identity.organization_id = "ORG-001";
    mission.identity.operator_id = "OP-001";
    mission.identity.mission_type = "survey";
    mission.identity.operational_status = "draft";

    mission.intent.objective = "survey an area";
    mission.intent.task_type = "survey";
    mission.task.description = "Complete the requested survey area.";
    mission.constraints.mandatory = {"restricted_airspace", "minimum_energy_reserve"};
    mission.constraints.optimization_criteria = {"energy", "completion_time"};

    mission.environment_snapshots.push_back({
        "ENV-001", "1", {{"wind", "weather-source", "2026-09-11T19:00:00Z", "valid", "fresh", "nominal"}}});
    mission.vehicle_assignments.push_back({"UAV-01", "CFG-01"});
    mission.payload_assignments.push_back({"PAYLOAD-01", "CFG-01"});
    mission.route_candidates.push_back({"CAND-01", "astar", "1"});
    mission.selected_solution = {"CAND-01", "INPUT-01", "astar-1", "energy-model-1", "preserve reserve"};
    mission.regulatory_plans.push_back({"FPL-01", "pending", "1"});
    mission.authorizations.push_back({"AUTH-01", "pending", "1"});
    mission.executable_packages.push_back({"PKG-01", "UAV-01", "1"});
    mission.execution_references.push_back({"FLIGHT-01", "UAV-01", "FCS-01", "C2-01"});

    assert(mission.identity.mission_id == "MISSION-001");
    assert(mission.identity.mission_version == "1");
    assert(mission.intent.objective == "survey an area");
    assert(mission.constraints.mandatory.size() == 2);
    assert(mission.environment_snapshots.size() == 1);
    assert(mission.vehicle_assignments.size() == 1);
    assert(mission.payload_assignments.size() == 1);
    assert(mission.route_candidates.size() == 1);
    assert(mission.selected_solution.candidate_id == "CAND-01");
    assert(mission.regulatory_plans.size() == 1);
    assert(mission.authorizations.size() == 1);
    assert(mission.executable_packages.size() == 1);
    assert(mission.execution_references.size() == 1);

    return 0;
}
