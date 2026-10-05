#pragma once
#include "model/route_model.hpp"
#include "vertical_launch_recovery.hpp"
#include <string>

namespace bluesky::planning {

struct VerticalRouteBuildInput {
    std::string route_id;
    std::string route_version;
    std::string mission_id;
    std::string mission_version;
    GeoPoint launch;
    GeoPoint recovery;
    double cruise_altitude_m{0.0};
    double transition_speed_mps{0.0};
    VerticalLaunchRecoveryInput launch_profile;
    VerticalLaunchRecoveryInput recovery_profile;
};

struct VerticalRouteBuildResult {
    bool feasible{false};
    Route route;
    VerticalProfileResult launch_profile;
    VerticalProfileResult recovery_profile;
    std::string calculation_version;
};

class VerticalRouteBuilder {
public:
    static VerticalRouteBuildResult build(
        const VerticalRouteBuildInput& input,
        const std::string& calculation_version);
};

} // namespace bluesky::planning
