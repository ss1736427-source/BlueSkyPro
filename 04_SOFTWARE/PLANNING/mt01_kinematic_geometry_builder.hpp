#pragma once

#include "coverage_path_planner.hpp"
#include "dubins_turn_connector.hpp"
#include "model/vehicle_motion_model.hpp"

#include <cstddef>
#include <string>
#include <vector>

namespace bluesky::planning {

enum class Mt01KinematicSegmentType { Coverage, ConnectorStraight, ConnectorTurn };

struct Mt01KinematicSegment {
    std::string segment_id;
    std::size_t from_waypoint_index{0};
    std::size_t to_waypoint_index{0};
    Mt01KinematicSegmentType type{Mt01KinematicSegmentType::Coverage};
    double distance_m{0.0};
    double nominal_speed_mps{0.0};
    double bank_angle_rad{0.0};
    double lateral_acceleration_mps2{0.0};
};

struct Mt01KinematicGeometryResult {
    bool feasible{false};
    double minimum_turn_radius_m{0.0};
    std::string vehicle_model_id;
    std::string vehicle_model_version;
    std::string vehicle_configuration_id;
    std::size_t connector_count{0};
    std::vector<GeoPoint> waypoints;
    // Exact capture-point index -> waypoint index in this geometry candidate.
    // Empty only when the coverage plan has no capture events.
    std::vector<std::size_t> capture_point_waypoint_indices;
    // One explicit dynamics record for every adjacent waypoint pair.
    std::vector<Mt01KinematicSegment> segments;
    std::vector<std::string> diagnostics;
};

// Builds an air-relative geometry candidate for fixed-wing or multirotor
// aircraft from ordered straight coverage legs. connector_speed_mps may be
// lower than coverage cruise; connector radius and connector segment speed
// are regenerated together. Zero selects the vehicle reference speed.
// This is not a complete/safety-attested Route:
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
        double maximum_sample_spacing_m = 2.0,
        double connector_speed_mps = 0.0);
};

} // namespace bluesky::planning
