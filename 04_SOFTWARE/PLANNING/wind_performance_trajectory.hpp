#pragma once

#include "model/route_model.hpp"
#include <string>
#include <vector>

namespace bluesky::planning {

enum class TrajectoryStatus {
    Feasible,
    Infeasible
};

enum class TrajectoryFindingCode {
    MissingWindSample,
    InvalidWindSample,
    WindExceedsTolerance,
    GroundSpeedUnavailable,
    InvalidPerformance,
    InsufficientEnergyReserve
};

struct WindSample {
    std::string segment_id;
    double north_mps{0.0};
    double east_mps{0.0};
    double up_mps{0.0};
};

struct TrajectoryPerformanceInput {
    std::string uav_id;
    std::string configuration_version;
    std::string performance_version;
    std::string wind_snapshot_id;
    std::string wind_snapshot_version;
    double cruise_airspeed_mps{0.0};
    double climb_rate_mps{0.0};
    double descent_rate_mps{0.0};
    double wind_tolerance_mps{0.0};
    double energy_per_horizontal_meter_wh{0.0};
    double energy_per_climb_meter_wh{0.0};
    double energy_per_descent_meter_wh{0.0};
    double usable_energy_wh{0.0};
    double reserve_requirement_wh{0.0};
    std::vector<WindSample> wind_samples;
};

struct TrajectoryFinding {
    TrajectoryFindingCode code;
    std::string segment_id;
    std::string detail;
};

struct TrajectoryPoint4D {
    std::string waypoint_id;
    GeoPoint position;
    double altitude_m{0.0};
    double elapsed_time_s{0.0};
    double cumulative_energy_wh{0.0};
};

struct TrajectorySegmentResult {
    std::string segment_id;
    double ground_speed_mps{0.0};
    double track_deg{0.0};
    double traversal_time_s{0.0};
    double energy_wh{0.0};
};

struct TrajectoryResult {
    TrajectoryStatus status{TrajectoryStatus::Infeasible};
    std::string route_id;
    std::string route_version;
    std::string uav_id;
    std::string configuration_version;
    std::string wind_snapshot_id;
    std::string wind_snapshot_version;
    std::string calculation_version;
    std::string dependency_identity;
    double total_time_s{0.0};
    double total_energy_wh{0.0};
    double remaining_energy_wh{0.0};
    std::vector<TrajectoryPoint4D> points;
    std::vector<TrajectorySegmentResult> segments;
    std::vector<TrajectoryFinding> findings;
};

class WindPerformanceTrajectory {
public:
    static TrajectoryResult calculate(
        const Route& route,
        const TrajectoryPerformanceInput& input,
        const std::string& calculation_version);
};

} // namespace bluesky::planning
