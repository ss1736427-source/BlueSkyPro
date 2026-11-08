#include "grib2_decoder.hpp"
#include "grib2_weather_pipeline.hpp"

#include <eccodes.h>

#include <cassert>
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

namespace {
bool writeWindMessage(const char* path, const char* shortName,
                      double value, const char* mode)
{
    codes_handle* handle = codes_handle_new_from_samples(nullptr, "regular_ll_sfc_grib2");
    if (!handle) return false;

    std::size_t nameLength = std::string(shortName).size();
    std::size_t levelTypeLength = std::string("heightAboveGround").size();
    const bool metadataOk =
        codes_set_string(handle, "shortName", shortName, &nameLength) == CODES_SUCCESS &&
        codes_set_string(handle, "typeOfLevel", "heightAboveGround",
                         &levelTypeLength) == CODES_SUCCESS &&
        codes_set_long(handle, "level", 50) == CODES_SUCCESS &&
        codes_set_long(handle, "dataDate", 20261108) == CODES_SUCCESS &&
        codes_set_long(handle, "dataTime", 600) == CODES_SUCCESS &&
        codes_set_long(handle, "forecastTime", 0) == CODES_SUCCESS;

    std::size_t count = 0;
    bool valuesOk = metadataOk &&
                    codes_get_size(handle, "values", &count) == CODES_SUCCESS;
    std::vector<double> values(count, value);
    if (valuesOk)
        valuesOk = codes_set_double_array(handle, "values", values.data(), values.size()) == CODES_SUCCESS;

    const bool writeOk = valuesOk &&
                         codes_write_message(handle, path, mode) == CODES_SUCCESS;
    codes_handle_delete(handle);
    return writeOk;
}
}

int main()
{
    const std::string path = "weather_decoder_fixture.grib2";
    std::remove(path.c_str());

    assert(writeWindMessage(path.c_str(), "u", 4.0, "w"));
    assert(writeWindMessage(path.c_str(), "v", 2.0, "a"));

    std::vector<bluesky::weather::WindField> fields;
    std::string error;
    const bool decoded = bluesky::weather::Grib2Decoder::decodeWindFile(path, fields, &error);
    assert(decoded && error.empty());
    assert(fields.size() == 1);
    assert(fields[0].level_agl_m == 50);
    assert(fields[0].valid_date_utc == 20261108);
    assert(fields[0].valid_time_utc == 600);
    assert(fields[0].rows >= 2 && fields[0].columns >= 2);
    assert(fields[0].eastward_mps.size() == fields[0].rows * fields[0].columns);
    assert(fields[0].northward_mps.size() == fields[0].rows * fields[0].columns);
    for (double value : fields[0].eastward_mps)
        assert(std::abs(value - 4.0) < 1e-6);
    for (double value : fields[0].northward_mps)
        assert(std::abs(value - 2.0) < 1e-6);

    {
        const auto& grid = fields.front();
        const std::size_t y = (grid.rows / 2 < grid.rows - 1) ? grid.rows / 2 : grid.rows - 2;
        const std::size_t x = grid.columns / 2;
        const double lat0 = grid.latitudes_deg[y];
        const double lat1 = grid.latitudes_deg[y + 1];
        const double lon = grid.longitudes_deg[x];

        bluesky::planning::Route route;
        route.lineage.route_id = "fixture-route";
        route.lineage.route_version = "1";
        route.waypoints = {
            {"A", {lat0, lon}, 50.0, true, bluesky::planning::AltitudeReference::AGL},
            {"B", {lat1, lon}, 50.0, true, bluesky::planning::AltitudeReference::AGL}
        };
        route.segments = {{"SEG-1", "A", "B", 0.0, 0.0}};

        bluesky::planning::TrajectoryPerformanceInput input;
        input.uav_id = "fixture-uav";
        input.configuration_version = "cfg-1";
        input.performance_version = "perf-1";
        input.cruise_airspeed_mps = 15.0;
        input.climb_rate_mps = 3.0;
        input.descent_rate_mps = 3.0;
        input.wind_tolerance_mps = 30.0;
        input.energy_per_horizontal_meter_wh = 0.0001;
        input.energy_per_climb_meter_wh = 0.01;
        input.energy_per_descent_meter_wh = 0.01;
        input.usable_energy_wh = 1000.0;
        input.reserve_requirement_wh = 10.0;

        const auto pipeline = bluesky::weather::Grib2WeatherPipeline::evaluateFile(
            path, route, "fixture-snapshot", "fixture-v1", input, "fixture-calc-v1");
        assert(pipeline.decode_succeeded);
        assert(pipeline.evaluation.weather_ready);
        assert(pipeline.evaluation.weather_error.empty());
        assert(pipeline.evaluation.trajectory.status == bluesky::planning::TrajectoryStatus::Feasible);
        assert(pipeline.evaluation.trajectory.wind_snapshot_id == "fixture-snapshot");
    }

    fields.clear();
    assert(!bluesky::weather::Grib2Decoder::decodeWindFile(
        "this-file-does-not-exist.grib2", fields, &error));
    assert(fields.empty() && !error.empty());

    std::remove(path.c_str());
    return 0;
}
