#include "mission_compiler.hpp"

#include <set>

namespace bluesky::planning {

namespace {

bool validate_mission(const Mission& mission, std::string& rejection_reason) {
    rejection_reason.clear();

    if (mission.identity.mission_id.empty()) {
        rejection_reason = "MISSION_ID_REQUIRED";
        return false;
    }
    if (mission.identity.mission_version.empty()) {
        rejection_reason = "MISSION_VERSION_REQUIRED";
        return false;
    }
    if (mission.vehicle_assignments.empty()) {
        rejection_reason = "VEHICLE_ASSIGNMENT_REQUIRED";
        return false;
    }
    if (mission.selected_solution.candidate_id.empty()) {
        rejection_reason = "SELECTED_SOLUTION_REQUIRED";
        return false;
    }

    bool selected_candidate_exists = false;
    for (const auto& candidate : mission.route_candidates) {
        if (candidate.candidate_id == mission.selected_solution.candidate_id) {
            selected_candidate_exists = true;
            break;
        }
    }
    if (!mission.route_candidates.empty() && !selected_candidate_exists) {
        rejection_reason = "SELECTED_ROUTE_CANDIDATE_NOT_FOUND";
        return false;
    }

    std::set<std::string> vehicle_ids;
    for (const auto& vehicle : mission.vehicle_assignments) {
        if (vehicle.vehicle_id.empty()) {
            rejection_reason = "VEHICLE_ID_REQUIRED";
            return false;
        }
        if (vehicle.configuration_version.empty()) {
            rejection_reason = "VEHICLE_CONFIGURATION_REQUIRED";
            return false;
        }
        if (!vehicle_ids.insert(vehicle.vehicle_id).second) {
            rejection_reason = "DUPLICATE_VEHICLE_ASSIGNMENT";
            return false;
        }
    }

    return true;
}

ExecutableMissionPackage make_package(const Mission& mission,
                                      const VehicleAssignment& vehicle) {
    ExecutableMissionPackage package;
    package.package_id = mission.identity.mission_id + "-PKG-" +
                         mission.identity.mission_version + "-" + vehicle.vehicle_id;
    package.vehicle_id = vehicle.vehicle_id;
    package.configuration_version = vehicle.configuration_version;
    package.package_version = mission.identity.mission_version;
    package.mission_id = mission.identity.mission_id;
    package.mission_version = mission.identity.mission_version;
    package.route_candidate_id = mission.selected_solution.candidate_id;
    package.algorithm_version = mission.selected_solution.algorithm_version;
    package.model_version = mission.selected_solution.model_version;
    package.regulatory_plan_id = mission.regulatory_plans.empty()
        ? "" : mission.regulatory_plans.front().plan_id;
    package.authorization_id = mission.authorizations.empty()
        ? "" : mission.authorizations.front().authorization_id;
    package.execution_steps = {"LOAD", "ARM_GATE", "EXECUTE", "COMPLETE"};
    return package;
}

} // namespace

bool MissionCompiler::compile(const Mission& mission,
                              std::vector<ExecutableMissionPackage>& packages,
                              std::string& rejection_reason) {
    packages.clear();
    if (!validate_mission(mission, rejection_reason)) {
        return false;
    }

    packages.reserve(mission.vehicle_assignments.size());
    for (const auto& vehicle : mission.vehicle_assignments) {
        packages.push_back(make_package(mission, vehicle));
    }
    return true;
}

bool MissionCompiler::compile(const Mission& mission,
                              ExecutableMissionPackage& package,
                              std::string& rejection_reason) {
    std::vector<ExecutableMissionPackage> packages;
    if (!compile(mission, packages, rejection_reason)) {
        return false;
    }
    if (packages.size() != 1) {
        rejection_reason = "MULTI_VEHICLE_REQUIRES_PACKAGE_COLLECTION";
        return false;
    }
    package = packages.front();
    return true;
}

} // namespace bluesky::planning
