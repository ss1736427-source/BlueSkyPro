#pragma once

#include "weather_trajectory_calculator.hpp"

#include <string>
#include <vector>

namespace bluesky::weather {

struct Grib2WeatherEvaluation {
    bool decode_succeeded{false};
    WeatherTrajectoryEvaluation evaluation;
};

// Immutable decoded forecast snapshot shared by every candidate in one planning run.
// A snapshot is usable only when decode_succeeded is true and fields is non-empty.
struct Grib2WeatherSnapshot {
    bool decode_succeeded{false};
    std::string snapshot_id;
    std::string snapshot_version;
    std::vector<WindField> fields;
    std::string error;
};

// End-to-end core pipeline: one or more GRIB2 products -> ecCodes decode ->
// valid-time/level merge and duplicate QC -> immutable snapshot or trajectory assessment.
class Grib2WeatherPipeline final {
public:
    // Decode all products once and return the exact immutable field set to be
    // shared by CandidateWeatherAssessor across every route candidate.
    static Grib2WeatherSnapshot decodeFiles(
        const std::vector<std::string>& grib2_paths,
        const std::string& snapshot_id,
        const std::string& snapshot_version);

    static Grib2WeatherEvaluation evaluateFile(
        const std::string& grib2_path,
        const bluesky::planning::Route& route,
        const std::string& snapshot_id,
        const std::string& snapshot_version,
        bluesky::planning::TrajectoryPerformanceInput input,
        const std::string& calculation_version,
        int maximum_vertical_gap_m = 100);

    // Each file may contain a different GFS forecast lead. Products are merged
    // only when their (valid UTC time, AGL level) identities are unique.
    static Grib2WeatherEvaluation evaluateFiles(
        const std::vector<std::string>& grib2_paths,
        const bluesky::planning::Route& route,
        const std::string& snapshot_id,
        const std::string& snapshot_version,
        bluesky::planning::TrajectoryPerformanceInput input,
        const std::string& calculation_version,
        int maximum_vertical_gap_m = 100);
};

} // namespace bluesky::weather
