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

Grib2WeatherSnapshot Grib2WeatherPipeline::decodeFiles(
    const std::vector<std::string>& grib2_paths,
    const std::string& snapshot_id,
    const std::string& snapshot_version)
{
    Grib2WeatherSnapshot snapshot;
    snapshot.snapshot_id=snapshot_id;
    snapshot.snapshot_version=snapshot_version;
    if (grib2_paths.empty()) {
        snapshot.error="at least one GRIB2 forecast product is required";
        return snapshot;
    }
    if (snapshot_id.empty() || snapshot_version.empty()) {
        snapshot.error="forecast snapshot identity and version are required";
        return snapshot;
    }

    std::set<std::tuple<int,int,int>> fieldIdentities;
    for (const auto& path:grib2_paths) {
        if (path.empty()) {
            snapshot.error="GRIB2 forecast product path is empty";
            return snapshot;
        }
        std::vector<WindField> decodedFields;
        std::string decodeError;
        if (!Grib2Decoder::decodeWindFile(path,decodedFields,&decodeError)) {
            snapshot.error="GRIB2 decode failed for "+path+": "+decodeError;
            return snapshot;
        }
        for (auto& field:decodedFields) {
            const auto identity=std::make_tuple(
                field.valid_date_utc,field.valid_time_utc,field.level_agl_m);
            if (!fieldIdentities.insert(identity).second) {
                snapshot.error="duplicate forecast field across GRIB2 products for valid date/time and AGL level";
                return snapshot;
            }
            snapshot.fields.push_back(std::move(field));
        }
    }
    if (snapshot.fields.empty()) {
        snapshot.error="GRIB2 products contain no decoded wind fields";
        return snapshot;
    }

    std::sort(snapshot.fields.begin(),snapshot.fields.end(),
        [](const WindField& a,const WindField& b) {
            if (a.valid_date_utc!=b.valid_date_utc)
                return a.valid_date_utc<b.valid_date_utc;
            if (a.valid_time_utc!=b.valid_time_utc)
                return a.valid_time_utc<b.valid_time_utc;
            return a.level_agl_m<b.level_agl_m;
        });
    snapshot.decode_succeeded=true;
    return snapshot;
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
    const auto snapshot=decodeFiles(grib2_paths,snapshot_id,snapshot_version);
    result.decode_succeeded=snapshot.decode_succeeded;
    if (!snapshot.decode_succeeded) {
        result.evaluation.weather_error=snapshot.error;
        return result;
    }
    result.evaluation=WeatherTrajectoryCalculator::evaluate(
        route,snapshot.fields,snapshot.snapshot_id,snapshot.snapshot_version,
        std::move(input),calculation_version,maximum_vertical_gap_m);
    return result;
}

} // namespace bluesky::weather