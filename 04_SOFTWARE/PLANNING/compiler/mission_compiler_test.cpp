#include "mission_compiler.hpp"
#include <iostream>

using namespace bluesky::planning;

int main() {
    Mission mission;
    mission.identity.mission_id = "MIS-001";
    mission.identity.mission_version = "1";
    mission.vehicle_assignments.push_back({"UAV-001", "CFG-1"});
    mission.selected_solution.candidate_id = "ROUTE-001";

    ExecutableMissionPackage package;
    std::string reason;

    if (!MissionCompiler::compile(mission, package, reason)) return 1;
    if (package.package_id != "MIS-001-PKG-1-UAV-001") return 2;
    if (package.vehicle_id != "UAV-001") return 3;
    if (package.configuration_version != "CFG-1") return 4;
    if (package.route_candidate_id != "ROUTE-001") return 5;
    if (package.execution_steps.size() != 4) return 6;

    Mission invalid = mission;
    invalid.identity.mission_id.clear();
    if (MissionCompiler::compile(invalid, package, reason)) return 7;
    if (reason != "MISSION_ID_REQUIRED") return 8;

    Mission group = mission;
    group.identity.mission_id = "MIS-GROUP-001";
    group.vehicle_assignments.push_back({"UAV-002", "CFG-2"});
    std::vector<ExecutableMissionPackage> packages;
    if (!MissionCompiler::compile(group, packages, reason)) return 9;
    if (packages.size() != 2) return 10;
    if (packages[0].vehicle_id != "UAV-001" || packages[1].vehicle_id != "UAV-002") return 11;
    if (packages[0].package_id != "MIS-GROUP-001-PKG-1-UAV-001") return 12;
    if (packages[1].package_id != "MIS-GROUP-001-PKG-1-UAV-002") return 13;

    Mission duplicate = group;
    duplicate.vehicle_assignments.push_back({"UAV-001", "CFG-X"});
    if (MissionCompiler::compile(duplicate, packages, reason)) return 14;
    if (reason != "DUPLICATE_VEHICLE_ASSIGNMENT") return 15;

    Mission no_config = mission;
    no_config.vehicle_assignments[0].configuration_version.clear();
    if (MissionCompiler::compile(no_config, packages, reason)) return 16;
    if (reason != "VEHICLE_CONFIGURATION_REQUIRED") return 17;

    std::cout << "PASS: mission compiler contract and multi-vehicle compilation\n";
    return 0;
}
