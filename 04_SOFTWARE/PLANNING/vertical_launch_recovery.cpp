#include "vertical_launch_recovery.hpp"
#include <algorithm>

namespace bluesky::planning {
VerticalProfileResult VerticalLaunchRecoveryPlanner::calculate(
    const VerticalLaunchRecoveryInput& input,
    const std::string& calculation_version) {
    VerticalProfileResult result;
    result.calculation_version = calculation_version;

    double required = std::max(
        input.terrain_elevation_m + input.minimum_clearance_m,
        input.regulatory_minimum_altitude_m);
    required = std::max(required, input.vehicle_minimum_transition_altitude_m);

    for (const auto& obstacle : input.obstacles) {
        required = std::max(required,
                            obstacle.clearance_altitude_m + input.minimum_clearance_m);
    }

    result.safe_transition_altitude_m = required;

    if (input.vehicle_max_altitude_m > 0.0 &&
        required > input.vehicle_max_altitude_m) {
        result.status = VerticalProfileStatus::Infeasible;
        for (const auto& obstacle : input.obstacles) {
            if (obstacle.clearance_altitude_m + input.minimum_clearance_m > input.vehicle_max_altitude_m)
                result.blocking_obstacle_ids.push_back(obstacle.obstacle_id);
        }
        if (result.blocking_obstacle_ids.empty())
            result.blocking_obstacle_ids.push_back("ALTITUDE_LIMIT");
        return result;
    }

    result.status = VerticalProfileStatus::Valid;
    return result;
}
} // namespace bluesky::planning
