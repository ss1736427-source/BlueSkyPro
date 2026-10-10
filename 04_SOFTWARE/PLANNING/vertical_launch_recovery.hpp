#pragma once
#include "model/route_model.hpp"
#include <string>
#include <vector>

namespace bluesky::planning {

enum class VerticalProfileStatus { Valid, Infeasible };

struct VerticalObstacle {
    std::string obstacle_id;
    double clearance_altitude_m{0.0};
};

struct VerticalLaunchRecoveryInput {
    GeoPoint point;
    double terrain_elevation_m{0.0};
    double minimum_clearance_m{0.0};
    double regulatory_minimum_altitude_m{0.0};
    double vehicle_minimum_transition_altitude_m{0.0};
    double vehicle_max_altitude_m{0.0};
    std::vector<VerticalObstacle> obstacles;
};

struct VerticalProfileResult {
    VerticalProfileStatus status{VerticalProfileStatus::Infeasible};
    double safe_transition_altitude_m{0.0};
    std::vector<std::string> blocking_obstacle_ids;
    std::string calculation_version;
};

class VerticalLaunchRecoveryPlanner {
public:
    static VerticalProfileResult calculate(
        const VerticalLaunchRecoveryInput& input,
        const std::string& calculation_version);
};

} // namespace bluesky::planning
