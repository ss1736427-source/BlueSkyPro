#pragma once

#include "weather_field.hpp"
#include "../PLANNING/wind_performance_trajectory.hpp"

#include <string>
#include <vector>

namespace bluesky::weather {

struct WeatherTrajectoryEvaluation {
    bool weather_ready{false};
    std::string weather_error;
    bluesky::planning::TrajectoryResult trajectory;
};

// The only supported entry point for a trajectory calculation that consumes
// decoded forecast wind. It fails closed before calling the existing physics
// calculator if any route segment lacks a valid wind sample.
class WeatherTrajectoryCalculator final {
public:
    static WeatherTrajectoryEvaluation evaluate(
        const bluesky::planning::Route& route,
        const std::vector<WindField>& fields,
        const std::string& snapshot_id,
        const std::string& snapshot_version,
        bluesky::planning::TrajectoryPerformanceInput input,
        const std::string& calculation_version,
        int maximum_vertical_gap_m = 100);
};

} // namespace bluesky::weather
