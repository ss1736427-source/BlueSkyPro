#pragma once

#include "model/mission_model.hpp"
#include "model/route_model.hpp"

#include <string>
#include <vector>

namespace bluesky::planning {

// Explicit inputs required to turn operator-authored line geometry into a
// structural route candidate. This does not check route/environment intersections,
// NOTAM legality, energy reserve, or release readiness. No defaults are provided.
struct ManualRouteBuildRequirements {
    std::string generator_id;
    std::string generator_version;
    std::string calculation_input_version;
    std::string terrain_snapshot_id;
    std::string airspace_snapshot_id;
    std::string weather_snapshot_id;
    std::string wind_snapshot_id;
    double minimum_target_agl_m{0.0};
    double maximum_target_agl_m{0.0};
    double minimum_clearance_m{0.0};
    double nominal_speed_mps{0.0};
};

struct ManualRouteBuildResult {
    bool valid{false}; // Structural candidate constructed; not a safety/release decision.
    bool flight_release_eligible{false}; // This builder can never authorize release.
    Route route;
    std::vector<std::string> errors;
};

// Supports ordered line geometry only. Area coverage, orbit, and unordered
// observation/inspection points require their own task-specific generators.
ManualRouteBuildResult buildManualRouteCandidate(
    const Mission& mission,
    const MissionGeometryObject& geometry,
    const ManualRouteBuildRequirements& requirements);

} // namespace bluesky::planning
