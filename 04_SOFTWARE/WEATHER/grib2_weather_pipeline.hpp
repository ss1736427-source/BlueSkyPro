#pragma once

#include "weather_trajectory_calculator.hpp"

#include <string>

namespace bluesky::weather {

struct Grib2WeatherEvaluation {
    bool decode_succeeded{false};
    WeatherTrajectoryEvaluation evaluation;
};

// End-to-end core pipeline: GRIB2 file -> ecCodes decode -> field QC and
// interpolation -> per-segment WindSample -> existing trajectory calculator.
class Grib2WeatherPipeline final {
public:
    static Grib2WeatherEvaluation evaluateFile(
        const std::string& grib2_path,
        const bluesky::planning::Route& route,
        const std::string& snapshot_id,
        const std::string& snapshot_version,
        bluesky::planning::TrajectoryPerformanceInput input,
        const std::string& calculation_version,
        int maximum_vertical_gap_m = 100);
};

} // namespace bluesky::weather
