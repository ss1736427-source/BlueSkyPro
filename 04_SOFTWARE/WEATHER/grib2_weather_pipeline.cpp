#include "grib2_weather_pipeline.hpp"
#include "grib2_decoder.hpp"

#include <utility>

namespace bluesky::weather {

Grib2WeatherEvaluation Grib2WeatherPipeline::evaluateFile(
    const std::string& grib2_path,
    const bluesky::planning::Route& route,
    const std::string& snapshot_id,
    const std::string& snapshot_version,
    bluesky::planning::TrajectoryPerformanceInput input,
    const std::string& calculation_version,
    int maximum_vertical_gap_m)
{
    Grib2WeatherEvaluation result;
    std::vector<WindField> fields;
    if (!Grib2Decoder::decodeWindFile(grib2_path, fields,
                                      &result.evaluation.weather_error)) {
        return result;
    }

    result.decode_succeeded = true;
    result.evaluation = WeatherTrajectoryCalculator::evaluate(
        route, fields, snapshot_id, snapshot_version, std::move(input),
        calculation_version, maximum_vertical_gap_m);
    return result;
}

} // namespace bluesky::weather
