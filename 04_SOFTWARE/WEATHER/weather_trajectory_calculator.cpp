#include "weather_trajectory_calculator.hpp"
#include "route_wind_adapter.hpp"

namespace bluesky::weather {

WeatherTrajectoryEvaluation WeatherTrajectoryCalculator::evaluate(
    const bluesky::planning::Route& route,
    const std::vector<WindField>& fields,
    const std::string& snapshot_id,
    const std::string& snapshot_version,
    bluesky::planning::TrajectoryPerformanceInput input,
    const std::string& calculation_version,
    int maximum_vertical_gap_m)
{
    WeatherTrajectoryEvaluation result;
    if (!RouteWindAdapter::apply(route, fields, snapshot_id, snapshot_version,
                                 input, &result.weather_error,
                                 maximum_vertical_gap_m)) {
        return result;
    }

    result.weather_ready = true;
    result.trajectory = bluesky::planning::WindPerformanceTrajectory::calculate(
        route, input, calculation_version);
    return result;
}

} // namespace bluesky::weather
