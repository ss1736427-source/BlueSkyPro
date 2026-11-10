#include "weather_field.hpp"
#include "route_wind_adapter.hpp"
#include "weather_trajectory_calculator.hpp"

#include <cassert>
#include <cmath>
#include <limits>
#include <string>
#include <vector>

using namespace bluesky::weather;
using namespace bluesky::planning;

static WindField fieldAt(int level, double u, double v, int validTime)
{
    WindField f;
    f.level_agl_m = level;
    f.valid_date_utc = 20261108;
    f.valid_time_utc = validTime;
    f.rows = 2;
    f.columns = 2;
    f.latitudes_deg = {0.0, 1.0};
    f.longitudes_deg = {10.0, 11.0};
    f.eastward_mps = {u, u, u, u};
    f.northward_mps = {v, v, v, v};
    return f;
}

static WindField field(int level,double u,double v)
{
    return fieldAt(level,u,v,600);
}

int main()
{
    const WindField low = field(50, 4.0, 2.0);
    const WindField mid = field(80, 7.0, 5.0);
    const WindField high = field(100, 9.0, 7.0);
    const WindField top = field(1000, 20.0, 18.0);

    const WindField nextLow=fieldAt(50,8.0,6.0,700);
    const WindField nextMid=fieldAt(80,11.0,9.0,700);
    const WindField nextHigh=fieldAt(100,13.0,11.0,700);
    const WindField nextTop=fieldAt(1000,24.0,22.0,700);
    const std::vector<WindField> timeFields{
        low,mid,high,top,nextLow,nextMid,nextHigh,nextTop
    };
    const auto timeInterpolated=WindInterpolator::atTimeAndAltitude(
        timeFields,0.5,10.5,65.0,1794119400LL);
    assert(timeInterpolated.valid);
    assert(std::abs(timeInterpolated.eastward_mps-7.5)<1e-9);
    assert(std::abs(timeInterpolated.northward_mps-5.5)<1e-9);
    const auto noTemporalExtrapolation=WindInterpolator::atTimeAndAltitude(
        timeFields,0.5,10.5,65.0,1794114000LL);
    assert(!noTemporalExtrapolation.valid);
    assert(noTemporalExtrapolation.quality==WindQuality::MissingTemporalCoverage);
    const auto excessiveTemporalGap=WindInterpolator::atTimeAndAltitude(
        {low,mid,high,top,fieldAt(50,8.0,6.0,1200),
         fieldAt(80,11.0,9.0,1200),fieldAt(100,13.0,11.0,1200),
         fieldAt(1000,24.0,22.0,1200)},
        0.5,10.5,65.0,1794119400LL,100,3600);
    assert(!excessiveTemporalGap.valid);
    assert(excessiveTemporalGap.quality==WindQuality::TemporalGapTooLarge);

    const auto horizontal = WindInterpolator::horizontal(low, 0.5, 10.5);
    assert(horizontal.valid);
    assert(std::abs(horizontal.eastward_mps - 4.0) < 1e-9);
    assert(std::abs(horizontal.northward_mps - 2.0) < 1e-9);

    const auto exact = WindInterpolator::atAltitude({low, mid, high, top}, 0.5, 10.5, 80.0);
    assert(exact.valid && std::abs(exact.eastward_mps - 7.0) < 1e-9);

    const auto vertical = WindInterpolator::atAltitude({low, mid, high, top}, 0.5, 10.5, 65.0);
    assert(vertical.valid);
    assert(std::abs(vertical.eastward_mps - 5.5) < 1e-9);
    assert(std::abs(vertical.northward_mps - 3.5) < 1e-9);

    const auto largeGap = WindInterpolator::atAltitude({low, mid, high, top}, 0.5, 10.5, 500.0);
    assert(!largeGap.valid && largeGap.quality == WindQuality::VerticalGapTooLarge);

    const auto extrapolation = WindInterpolator::atAltitude({low, mid}, 0.5, 10.5, 30.0);
    assert(!extrapolation.valid && extrapolation.quality == WindQuality::MissingVerticalCoverage);

    const auto outside = WindInterpolator::horizontal(low, 2.0, 10.5);
    assert(!outside.valid && outside.quality == WindQuality::OutsideGrid);

    WindField missing = low;
    missing.eastward_mps[0] = std::numeric_limits<double>::quiet_NaN();
    const auto missingResult = WindInterpolator::horizontal(missing, 0.5, 10.5);
    assert(!missingResult.valid && missingResult.quality == WindQuality::MissingGridValue);

    Route route;
    route.lineage.route_id = "route-1";
    route.lineage.route_version = "v1";
    route.waypoints = {
        {"A", {0.25, 10.25}, 50.0, true, AltitudeReference::AGL},
        {"B", {0.75, 10.75}, 80.0, true, AltitudeReference::AGL}
    };
    route.segments = {{"S1", "A", "B", 0.0, 0.0}};

    TrajectoryPerformanceInput input;
    input.wind_snapshot_id = "old";
    input.wind_snapshot_version = "old-v";
    input.wind_samples = {{"OLD", 1.0, 1.0, 0.0}};
    std::string error;
    assert(RouteWindAdapter::apply(route, {low, mid, high, top},
                                   "snapshot-20261108-06", "gfs-0p25-v1",
                                   input, &error));
    assert(input.wind_snapshot_id == "snapshot-20261108-06");
    assert(input.wind_snapshot_version == "gfs-0p25-v1");
    assert(input.wind_samples.size() == 1);
    assert(input.wind_samples[0].segment_id == "S1");
    assert(std::abs(input.wind_samples[0].north_mps - 3.5) < 1e-9);
    assert(std::abs(input.wind_samples[0].east_mps - 5.5) < 1e-9);
    assert(input.wind_samples[0].up_mps == 0.0);

    TrajectoryPerformanceInput performanceInput;
    performanceInput.uav_id = "uav-1";
    performanceInput.mission_start_utc = "2026-11-08T06:00:00Z";
    performanceInput.configuration_version = "cfg-1";
    performanceInput.performance_version = "perf-1";
    performanceInput.cruise_airspeed_mps = 15.0;
    performanceInput.climb_rate_mps = 3.0;
    performanceInput.descent_rate_mps = 3.0;
    performanceInput.wind_tolerance_mps = 30.0;
    performanceInput.energy_per_horizontal_meter_wh = 0.001;
    performanceInput.energy_per_climb_meter_wh = 0.01;
    performanceInput.energy_per_descent_meter_wh = 0.01;
    performanceInput.usable_energy_wh = 100.0;
    performanceInput.reserve_requirement_wh = 10.0;

    const auto incompleteEvaluation = WeatherTrajectoryCalculator::evaluate(
        route, timeFields, "snapshot-20261108-06", "gfs-0p25-v1",
        performanceInput, "weather-trajectory-test-v1");
    assert(!incompleteEvaluation.weather_ready);
    assert(incompleteEvaluation.weather_error.find("trajectory geometry is incomplete")
           != std::string::npos);

    route.trajectory_geometry_complete = true;
    const auto evaluated = WeatherTrajectoryCalculator::evaluate(
        route, timeFields, "snapshot-20261108-06", "gfs-0p25-v1",
        performanceInput, "weather-trajectory-test-v1");
    assert(evaluated.weather_ready);
    assert(evaluated.weather_error.empty());
    assert(evaluated.trajectory.status == TrajectoryStatus::Feasible);
    assert(evaluated.trajectory.wind_snapshot_id == "snapshot-20261108-06");
    assert(evaluated.trajectory.wind_snapshot_version == "gfs-0p25-v1");

    const auto oldSnapshot = input.wind_snapshot_id;
    route.waypoints[1].altitude_reference = AltitudeReference::Unknown;
    assert(!RouteWindAdapter::apply(route, {low, mid, high, top},
                                    "new-snapshot", "new-v", input, &error));
    assert(input.wind_snapshot_id == oldSnapshot);

    return 0;
}
