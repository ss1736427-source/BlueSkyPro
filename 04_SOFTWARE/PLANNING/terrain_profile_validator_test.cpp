#include "terrain_profile_validator.hpp"

#include <cassert>
#include <cmath>
#include <limits>

using namespace bluesky::planning;

static Route makeRoute()
{
    Route route;
    route.lineage = {"TERRAIN-ROUTE","2","TERRAIN-MISSION","3","test","1","INPUT-9"};
    route.environment.terrain_snapshot_id = "DEM-TEST-1";
    route.constraints.minimum_clearance_m = 30.0;
    route.trajectory_geometry_complete = true;
    route.waypoints = {
        {"A",{60.0,25.0},60.0,true,AltitudeReference::AGL},
        {"B",{60.001,25.001},60.0,false,AltitudeReference::AGL}
    };
    route.segments = {{"AB","A","B",124.4,15.0}};
    return route;
}

static TerrainDemGrid makeDem()
{
    TerrainDemGrid dem;
    dem.snapshot_id="DEM-TEST-1";
    dem.source_id="TEST-DEM-PROVIDER";
    dem.source_version="1";
    dem.dataset_id="TEST-DATASET";
    dem.dataset_version="2026-11";
    dem.horizontal_crs="EPSG:4326";
    dem.vertical_datum="EGM96";
    dem.origin_latitude_deg=59.995;
    dem.origin_longitude_deg=24.995;
    dem.latitude_step_deg=0.0001;
    dem.longitude_step_deg=0.0001;
    dem.horizontal_resolution_m=10.0;
    dem.vertical_accuracy_m=2.0;
    dem.rows=101;
    dem.columns=101;
    dem.elevation_m.assign(dem.rows*dem.columns,100.0);
    return dem;
}

int main()
{
    auto route=makeRoute();
    auto dem=makeDem();
    const auto pass=TerrainProfileValidator::evaluate(route,dem);
    assert(pass.validation.passed);
    assert(pass.validation.complete_trajectory_coverage);
    assert(pass.validation.profile_sample_count>2);
    assert(pass.validation.minimum_observed_clearance_m>=30.0);
    assert(makeTerrainValidationEvidence(route,pass.snapshot,pass.validation).status
        ==MissionLifecycleValidationStatus::Passed);

    auto ridge=dem;
    for(std::size_t row=0;row<ridge.rows;++row)
        ridge.elevation_m[row*ridge.columns+55]=145.0;
    const auto ridgeResult=TerrainProfileValidator::evaluate(route,ridge);
    assert(!ridgeResult.validation.passed);
    assert(!ridgeResult.findings.empty());

    auto noData=dem;
    noData.elevation_m[55*noData.columns+55]=std::numeric_limits<double>::quiet_NaN();
    const auto noDataResult=TerrainProfileValidator::evaluate(route,noData);
    assert(!noDataResult.validation.passed);

    auto stale=dem;
    stale.snapshot_id="DEM-OTHER";
    const auto staleResult=TerrainProfileValidator::evaluate(route,stale);
    assert(!staleResult.validation.passed);

    auto unknownDatum=dem;
    unknownDatum.vertical_datum="UNKNOWN";
    assert(!TerrainProfileValidator::evaluate(route,unknownDatum).validation.passed);

    auto incomplete=route;
    incomplete.trajectory_geometry_complete=false;
    assert(!TerrainProfileValidator::evaluate(incomplete,dem).validation.passed);

    auto badDatum=route;
    badDatum.waypoints[0].altitude_reference=AltitudeReference::AMSL;
    assert(!TerrainProfileValidator::evaluate(badDatum,dem).validation.passed);

    return 0;
}
