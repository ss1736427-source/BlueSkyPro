#pragma once
#include <string>
#include <vector>
namespace bluesky::planning {
enum class SpeedProfileStatus { Feasible, Infeasible };
struct SpeedProfileSegmentInput { std::string segment_id; double distance_m{0.0}; double minimum_speed_mps{0.0}; double maximum_speed_mps{0.0}; };
struct SpeedProfileInput {
 std::string vehicle_model_id, vehicle_model_version, configuration_id, speed_reference;
 bool longitudinal_dynamics_validated{false}; std::string longitudinal_dynamics_source;
 double maximum_acceleration_mps2{0.0}, maximum_deceleration_mps2{0.0};
 double initial_speed_mps{0.0}, final_speed_mps{0.0};
 std::vector<SpeedProfileSegmentInput> segments;
};
struct SpeedProfileSegmentResult {
 std::string segment_id; double distance_m{0.0};
 double entry_speed_mps{0.0}, exit_speed_mps{0.0};
 double maximum_reachable_speed_mps{0.0}, acceleration_distance_m{0.0};
 double deceleration_distance_m{0.0}, cruise_distance_m{0.0};
 double maximum_acceleration_mps2{0.0}, maximum_deceleration_mps2{0.0};
};
struct SpeedProfileResult {
 SpeedProfileStatus status{SpeedProfileStatus::Infeasible};
 std::string vehicle_model_id, vehicle_model_version, configuration_id, speed_reference, dynamics_source;
 std::vector<SpeedProfileSegmentResult> segments; std::vector<std::string> findings;
};
class VariableSpeedProfileSolver { public: static SpeedProfileResult calculate(const SpeedProfileInput& input); };
} // namespace bluesky::planning
