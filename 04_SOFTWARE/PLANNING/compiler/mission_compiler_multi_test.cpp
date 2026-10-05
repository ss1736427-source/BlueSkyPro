#include "mission_compiler.hpp"
#include <iostream>

using namespace bluesky::planning;

int main() {
    Mission mission;
    mission.identity.mission_id = "MIS-GROUP-001";
    mission.identity.mission_version = "1";
    mission.vehicle_assignments.push_back({"UAV-001", "CFG-1"});
    mission.vehicle_assignments.push_back({"UAV-002", "CFG-2"});
    mission.selected_solution.candidate_id = "ROUTE-001";

    std::vector<ExecutableMissionPackage> packages;
    std::string reason;
    if (!MissionCompiler::compile(mission, packages, reason)) return 1;
    if (packages.size() != 2) return 2;
    if (packages[0].vehicle_id != "UAV-001" || packages[1].vehicle_id != "UAV-002") return 3;
    if (packages[0].package_id != "MIS-GROUP-001-PKG-1-UAV-001") return 4;
    if (packages[1].package_id != "MIS-GROUP-001-PKG-1-UAV-002") return 5;

    std::cout << "PASS: deterministic multi-vehicle package emission\n";
    return 0;
}
