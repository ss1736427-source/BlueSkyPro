#include "grib2_decoder.hpp"
#include "grib2_weather_pipeline.hpp"

#include <eccodes.h>

#include <cassert>
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

namespace {
bool writeWindMessage(const char* path,const char* shortName,
                      double value,const char* mode,long forecastHour)
{
    codes_handle* handle=codes_handle_new_from_samples(nullptr,"regular_ll_sfc_grib2");
    if (!handle) return false;
    std::size_t nameLength=std::string(shortName).size();
    std::size_t levelTypeLength=std::string("heightAboveGround").size();
    const bool metadataOk=
        codes_set_string(handle,"shortName",shortName,&nameLength)==CODES_SUCCESS &&
        codes_set_string(handle,"typeOfLevel","heightAboveGround",&levelTypeLength)==CODES_SUCCESS &&
        codes_set_long(handle,"level",50)==CODES_SUCCESS &&
        codes_set_long(handle,"dataDate",20261108)==CODES_SUCCESS &&
        codes_set_long(handle,"dataTime",600)==CODES_SUCCESS &&
        codes_set_long(handle,"forecastTime",forecastHour)==CODES_SUCCESS;
    std::size_t count=0;
    bool valuesOk=metadataOk && codes_get_size(handle,"values",&count)==CODES_SUCCESS;
    std::vector<double> values(count,value);
    if (valuesOk)
        valuesOk=codes_set_double_array(handle,"values",values.data(),values.size())==CODES_SUCCESS;
    const bool writeOk=valuesOk && codes_write_message(handle,path,mode)==CODES_SUCCESS;
    codes_handle_delete(handle);
    return writeOk;
}
}

int main()
{
    const std::string pathF000="weather_decoder_fixture_f000.grib2";
    const std::string pathF001="weather_decoder_fixture_f001.grib2";
    std::remove(pathF000.c_str());
    std::remove(pathF001.c_str());

    assert(writeWindMessage(pathF000.c_str(),"u",4.0,"w",0));
    assert(writeWindMessage(pathF000.c_str(),"v",2.0,"a",0));
    assert(writeWindMessage(pathF001.c_str(),"u",6.0,"w",1));
    assert(writeWindMessage(pathF001.c_str(),"v",3.0,"a",1));

    std::vector<bluesky::weather::WindField> fields;
    std::string error;
    const bool decoded=bluesky::weather::Grib2Decoder::decodeWindFile(pathF000,fields,&error);
    assert(decoded && error.empty());
    assert(fields.size()==1);
    assert(fields[0].level_agl_m==50);
    assert(fields[0].valid_date_utc==20261108);
    assert(fields[0].valid_time_utc==600);
    assert(fields[0].rows>=2 && fields[0].columns>=2);
    assert(fields[0].eastward_mps.size()==fields[0].rows*fields[0].columns);
    assert(fields[0].northward_mps.size()==fields[0].rows*fields[0].columns);
    for (double value:fields[0].eastward_mps) assert(std::abs(value-4.0)<1e-6);
    for (double value:fields[0].northward_mps) assert(std::abs(value-2.0)<1e-6);

    {
        const auto& grid=fields.front();
        const std::size_t y=(grid.rows/2<grid.rows-1)?grid.rows/2:grid.rows-2;
        const std::size_t x=grid.columns/2;
        const double lat0=grid.latitudes_deg[y];
        const double lat1=lat0+0.00001;
        const double lon=grid.longitudes_deg[x];

        bluesky::planning::Route route;
        route.lineage.route_id="fixture-route";
        route.lineage.route_version="1";
        route.waypoints={
            {"A",{lat0,lon},50.0,true,bluesky::planning::AltitudeReference::AGL},
            {"B",{lat1,lon},50.0,true,bluesky::planning::AltitudeReference::AGL}
        };
        route.segments={{"SEG-1","A","B",0.0,0.0}};
        route.trajectory_geometry_complete=true;

        bluesky::planning::TrajectoryPerformanceInput input;
        input.uav_id="fixture-uav";
        input.mission_start_utc="2026-11-08T06:00:00Z";
        input.configuration_version="cfg-1";
        input.performance_version="perf-1";
        input.cruise_airspeed_mps=15.0;
        input.climb_rate_mps=3.0;
        input.descent_rate_mps=3.0;
        input.wind_tolerance_mps=30.0;
        input.energy_per_horizontal_meter_wh=0.0001;
        input.energy_per_climb_meter_wh=0.01;
        input.energy_per_descent_meter_wh=0.01;
        input.usable_energy_wh=1000.0;
        input.reserve_requirement_wh=10.0;

        const auto pipeline=bluesky::weather::Grib2WeatherPipeline::evaluateFiles(
            {pathF000,pathF001},route,"fixture-snapshot","fixture-v1",input,"fixture-calc-v1");
        assert(pipeline.decode_succeeded);
        assert(pipeline.evaluation.weather_ready);
        assert(pipeline.evaluation.weather_error.empty());
        assert(pipeline.evaluation.trajectory.status==bluesky::planning::TrajectoryStatus::Feasible);
        assert(pipeline.evaluation.trajectory.wind_snapshot_id=="fixture-snapshot");

        const auto duplicate=bluesky::weather::Grib2WeatherPipeline::evaluateFiles(
            {pathF000,pathF000},route,"fixture-snapshot","fixture-v1",input,"fixture-calc-v1");
        assert(!duplicate.decode_succeeded);
        assert(duplicate.evaluation.weather_error.find("duplicate forecast field")!=std::string::npos);

        const auto single=bluesky::weather::Grib2WeatherPipeline::evaluateFile(
            pathF000,route,"fixture-snapshot","fixture-v1",input,"fixture-calc-v1");
        assert(single.decode_succeeded && single.evaluation.weather_ready);
    }

    fields.clear();
    assert(!bluesky::weather::Grib2Decoder::decodeWindFile(
        "this-file-does-not-exist.grib2",fields,&error));
    assert(fields.empty() && !error.empty());

    std::remove(pathF000.c_str());
    std::remove(pathF001.c_str());
    return 0;
}
