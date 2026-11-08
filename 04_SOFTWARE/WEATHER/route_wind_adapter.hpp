#pragma once

#include "weather_field.hpp"
#include "../PLANNING/wind_performance_trajectory.hpp"

#include <string>
#include <vector>

namespace bluesky::weather {

// Builds all segment samples first and mutates the trajectory input only when
// every route segment has valid spatial/vertical wind coverage.
class RouteWindAdapter final {
public:
    static bool apply(const bluesky::planning::Route& route,
                      const std::vector<WindField>& fields,
                      const std::string& snapshot_id,
                      const std::string& snapshot_version,
                      bluesky::planning::TrajectoryPerformanceInput& input,
                      std::string* error = nullptr,
                      int maximum_vertical_gap_m = 100);
};

} // namespace bluesky::weather
