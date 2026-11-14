#include "../WEATHER/weather_trajectory_calculator.hpp"

#include <cassert>
#include <cmath>
#include <string>
#include <vector>

using namespace bluesky::planning;
using namespace bluesky::weather;

namespace {
WindField field(int validTime, int level)
{
    WindField value;
    value.level_agl_m=level;
    value.valid_date_utc=20261108;
    value.valid_time_utc=validTime;
    value.rows=2; value.columns=2;
    value.latitudes_deg={0.0,1.0};
    value.longitudes_deg={10.0,11.0};
    value.eastward_mps={0.0,0.0,0.0,0.0};
    value.northward_mps={0.0,0.0,0.0,0.0};
    return value;
}
}

int main()
{
    Route route;
    route.lineage.mission_id="MT01-CAMERA-WEATHER";
    route.lineage.mission_version="1";
    route.lineage.route_id="ROUTE-01";
    route.lineage.route_version="1";
    route.lineage.calculation_input_version="INPUT-01";
    route.trajectory_geometry_complete=true;
    RouteWaypoint start;
    start.waypoint_id="START"; start.position={0.25,10.25};
    start.altitude_m=65.0; start.altitude_reference=AltitudeReference::AGL;
    RouteWaypoint finish=start;
    finish.waypoint_id="FINISH"; finish.position={0.25,10.45};
    route.waypoints={start,finish};
    route.segments={{"COV-LONG","START","FINISH",22239.0,10.0}};

    TrajectoryPerformanceInput input;
    input.uav_id="UAV-01"; input.configuration_version="CFG-01";
    input.performance_version="PERF-01";
    input.mission_start_utc="2026-11-08T06:00:00Z";
    input.cruise_airspeed_mps=10.0; input.climb_rate_mps=2.0;
    input.descent_rate_mps=2.0; input.wind_tolerance_mps=30.0;
    input.energy_per_horizontal_meter_wh=0.0001;
    input.energy_per_climb_meter_wh=0.01; input.energy_per_descent_meter_wh=0.01;
    input.payload_power_w=40.0; input.usable_energy_wh=1000.0;
    input.reserve_requirement_wh=10.0;
    input.camera_configuration_id="CAM-01"; input.camera_gsd_m_px=0.02;
    input.camera_forward_trigger_spacing_m=12.0;
    input.camera_minimum_trigger_interval_s=0.7;
    input.camera_exposure_time_s=1.0/2000.0;
    input.camera_maximum_motion_blur_px=0.5;
    input.camera_capture_segment_ids={"COV-LONG"};

    const std::vector<WindField> fields={
        field(600,50),field(600,80),
        field(700,50),field(700,80),
        field(800,50),field(800,80)};
    const auto result=WeatherTrajectoryCalculator::evaluate(
        route,fields,"GFS-SNAPSHOT","GFS-V1",input,"MT01-WEATHER-TEST");
    assert(result.weather_ready);
    assert(result.trajectory.status==TrajectoryStatus::Feasible);
    assert(result.trajectory.segments.size()>1);
    for (const auto& segment:result.trajectory.segments) {
        assert(segment.camera_trigger_interval_s>=0.7);
        assert(segment.camera_motion_blur_px<=0.5);
        assert(segment.camera_trigger_interval_s>0.0);
    }

    auto missingCameraSegment=input;
    missingCameraSegment.camera_capture_segment_ids={"UNKNOWN-SEGMENT"};
    const auto missing=WeatherTrajectoryCalculator::evaluate(
        route,fields,"GFS-SNAPSHOT","GFS-V1",missingCameraSegment,"MT01-WEATHER-TEST");
    assert(!missing.weather_ready);
    assert(missing.weather_error.find("camera capture segment identity")!=std::string::npos);
    return 0;
}
