#pragma once

#include <string>
#include <vector>

#include "../model/mission_model.hpp"

namespace bluesky::planning {

struct ExecutableMissionPackage {
    std::string package_id;
    std::string vehicle_id;
    std::string configuration_version;
    std::string package_version;
    std::string mission_id;
    std::string mission_version;
    std::string route_candidate_id;
    std::string algorithm_version;
    std::string model_version;
    std::string regulatory_plan_id;
    std::string authorization_id;
    std::vector<std::string> execution_steps;
};

class MissionCompiler {
public:
    static bool compile(const Mission& mission,
                        std::vector<ExecutableMissionPackage>& packages,
                        std::string& rejection_reason);

    static bool compile(const Mission& mission,
                        ExecutableMissionPackage& package,
                        std::string& rejection_reason);
};

} // namespace bluesky::planning
