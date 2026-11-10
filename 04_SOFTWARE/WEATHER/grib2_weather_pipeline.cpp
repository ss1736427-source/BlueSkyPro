#include "grib2_weather_pipeline.hpp"
#include "grib2_decoder.hpp"

#include <algorithm>
#include <set>
#include <tuple>
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
    return evaluateFiles({grib2_path},route,snapshot_id,snapshot_version,
                         std::move(input),calculation_version,maximum_vertical_gap_m);
}

Grib2WeatherEvaluation Grib2WeatherPipeline::evaluateFiles(
    const std::vector<std::string>& grib2_paths,
    const bluesky::planning::Route& route,
    const std::string& snapshot_id,
    const std::string& snapshot_version,
    bluesky::planning::TrajectoryPerformanceInput input,
    const std::string& calculation_version,
    int maximum_vertical_gap_m)
{
    Grib2WeatherEvaluation result;
    if (grib2_paths.empty()) {
        result.evaluation.weather_error="at least one GRIB2 forecast product is required";
        return result;
    }

    std::vector<WindField> mergedFields;
    std::set<std::tuple<int,int,int>> fieldIdentities;
    for (const auto& path:grib2_paths) {
        if (path.empty()) {
            result.evaluation.weather_error="GRIB2 forecast product path is empty";
            return result;
        }
        std::vector<WindField> decodedFields;
        std::string decodeError;
        if (!Grib2Decoder::decodeWindFile(path,decodedFields,&decodeError)) {
            result.evaluation.weather_error="GRIB2 decode failed for "+path+": "+decodeError;
            return result;
        }
        for (auto& field:decodedFields) {
            const auto identity=std::make_tuple(
                field.valid_date_utc,field.valid_time_utc,field.level_agl_m);
            if (!fieldIdentities.insert(identity).second) {
                result.evaluation.weather_error=
                    "duplicate forecast field across GRIB2 products for valid date/time and AGL level";
                return result;
            }
            mergedFields.push_back(std::move(field));
        }
    }
    if (mergedFields.empty()) {
        result.evaluation.weather_error="GRIB2 products contain no decoded wind fields";
        return result;
    }

    std::sort(mergedFields.begin(),mergedFields.end(),
        [](const WindField& a,const WindField& b) {
            if (a.valid_date_utc!=b.valid_date_utc)
                return a.valid_date_utc<b.valid_date_utc;
            if (a.valid_time_utc!=b.valid_time_utc)
                return a.valid_time_utc<b.valid_time_utc;
            return a.level_agl_m<b.level_agl_m;
        });

    result.decode_succeeded=true;
    result.evaluation=WeatherTrajectoryCalculator::evaluate(
        route,mergedFields,snapshot_id,snapshot_version,std::move(input),
        calculation_version,maximum_vertical_gap_m);
    return result;
}

} // namespace bluesky::weather
