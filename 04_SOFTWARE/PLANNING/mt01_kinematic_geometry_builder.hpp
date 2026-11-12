#pragma once

#include "coverage_path_planner.hpp"
#include "dubins_turn_connector.hpp"
#include "model/vehicle_motion_model.hpp"

#include <string>
#include <vector>

namespace bluesky::planning {

struct Mt01KinematicGeometryResult {
    bool feasible{false};
    double minimum_turn_radius_m{0.0};
    std::string vehicle_model_id;
    std::string vehicle_model_version;
    std::string vehicle_configuration_id;
    std::size_t connector_count{0};
    std::vector<GeoPoint> waypoints;
    std::vector<std::string> diagnostics;
};

// Builds an air-relative fixed-wing geometry candidate from the ordered
// straight coverage legs. This is not a complete/safety-attested Route:
// DEM, wind-ground-track, NOTAM/airspace, obstacles, energy and route hard gates
// remain separate requirements.
class Mt01KinematicGeometryBuilder {
public:
    static Mt01KinematicGeometryResult build(
        const CoveragePlanResult& coverage,
        const VehicleMotionModel& vehicle,
        const std::string& assigned_vehicle_id,
        const std::string& assigned_configuration_id,
        double coverage_cruise_speed_mps,
        const std::vector<std::vector<GeoPoint>>& exclusion_areas,
        double maximum_sample_spacing_m = 2.0);
};

} // namespace bluesky::planning
