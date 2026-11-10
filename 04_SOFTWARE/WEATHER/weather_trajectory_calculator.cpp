#include "weather_trajectory_calculator.hpp"
#include "route_wind_adapter.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <unordered_map>
#include <vector>

namespace bluesky::weather {
namespace {
constexpr int kMaximumIterations=12;
constexpr double kConvergenceToleranceSeconds=1.0;
constexpr double kDamping=0.5;
constexpr std::int64_t kMaximumForecastGapSeconds=10800;

bool parseUtc(const std::string& value,std::int64_t& seconds)
{
    if (value.size()!=20 || value[4]!='-' || value[7]!='-' || value[10]!='T'
        || value[13]!=':' || value[16]!=':' || value[19]!='Z') return false;
    const auto read=[&value](std::size_t at,std::size_t count,int& result) {
        result=0;
        for (std::size_t i=at;i<at+count;++i) {
            if (value[i]<'0' || value[i]>'9') return false;
            result=result*10+value[i]-'0';
        }
        return true;
    };
    int year=0,month=0,day=0,hour=0,minute=0,second=0;
    if (!read(0,4,year)||!read(5,2,month)||!read(8,2,day)
        ||!read(11,2,hour)||!read(14,2,minute)||!read(17,2,second)) return false;
    static constexpr int monthDays[]={31,28,31,30,31,30,31,31,30,31,30,31};
    if (year<1970 || month<1 || month>12 || hour>23 || minute>59 || second>59)
        return false;
    const bool leap=(year%4==0 && year%100!=0) || year%400==0;
    if (day<1 || day>monthDays[month-1]+(month==2 && leap ? 1 : 0)) return false;
    int y=year-(month<=2);
    const int era=(y>=0 ? y : y-399)/400;
    const unsigned yoe=static_cast<unsigned>(y-era*400);
    const int shifted=month+(month>2 ? -3 : 9);
    const unsigned doy=(153*static_cast<unsigned>(shifted)+2)/5
                       +static_cast<unsigned>(day)-1;
    const unsigned doe=yoe*365+yoe/4-yoe/100+doy;
    const std::int64_t days=static_cast<std::int64_t>(era)*146097
                            +static_cast<std::int64_t>(doe)-719468;
    seconds=days*86400+hour*3600+minute*60+second;
    return true;
}

double distanceMeters(const bluesky::planning::GeoPoint& a,
                      const bluesky::planning::GeoPoint& b)
{
    constexpr double pi=3.14159265358979323846, earth=6371000.0;
    const double lat1=a.latitude_deg*pi/180.0,lat2=b.latitude_deg*pi/180.0;
    const double north=(lat2-lat1)*earth;
    const double east=(b.longitude_deg-a.longitude_deg)*pi/180.0
                     *std::cos((lat1+lat2)*0.5)*earth;
    return std::hypot(north,east);
}

bool initialDurations(const bluesky::planning::Route& route,
                      const bluesky::planning::TrajectoryPerformanceInput& input,
                      std::vector<double>& durations,std::string& error)
{
    using namespace bluesky::planning;
    if (!std::isfinite(input.cruise_airspeed_mps)||input.cruise_airspeed_mps<=0.0
        ||!std::isfinite(input.climb_rate_mps)||input.climb_rate_mps<=0.0
        ||!std::isfinite(input.descent_rate_mps)||input.descent_rate_mps<=0.0) {
        error="positive finite airspeed, climb rate and descent rate are required";
        return false;
    }
    std::unordered_map<std::string,const RouteWaypoint*> points;
    for (const auto& point:route.waypoints) points.emplace(point.waypoint_id,&point);
    durations.clear();
    for (const auto& segment:route.segments) {
        const auto from=points.find(segment.from_waypoint_id);
        const auto to=points.find(segment.to_waypoint_id);
        if (from==points.end()||to==points.end()) {
            error="route segment references an unknown waypoint: "+segment.segment_id;
            return false;
        }
        const double horizontal=distanceMeters(from->second->position,to->second->position);
        const double dz=to->second->altitude_m-from->second->altitude_m;
        double duration=horizontal>1e-9 ? horizontal/input.cruise_airspeed_mps : 0.0;
        if (std::abs(dz)>1e-9) {
            const double rate=dz>0.0 ? input.climb_rate_mps : input.descent_rate_mps;
            duration=std::max(duration,std::abs(dz)/rate);
        }
        if (!std::isfinite(duration)||duration<=0.0) {
            error="route segment has no positive estimated traversal time: "+segment.segment_id;
            return false;
        }
        durations.push_back(duration);
    }
    return !durations.empty();
}

std::vector<double> midpointOffsets(const std::vector<double>& durations)
{
    std::vector<double> offsets;
    offsets.reserve(durations.size());
    double elapsed=0.0;
    for (double duration:durations) {
        offsets.push_back(elapsed+duration*0.5);
        elapsed+=duration;
    }
    return offsets;
}

bool buildTimeDependentWindSamples(
    const bluesky::planning::Route& route,const std::vector<WindField>& fields,
    std::int64_t missionStart,const std::vector<double>& offsets,
    int maximumVerticalGap,bluesky::planning::TrajectoryPerformanceInput& input,
    std::string& error)
{
    using namespace bluesky::planning;
    if (fields.empty() || offsets.size()!=route.segments.size()) {
        error="forecast fields or route midpoint-time list are incomplete";
        return false;
    }
    std::unordered_map<std::string,const RouteWaypoint*> points;
    for (const auto& point:route.waypoints) {
        if (point.altitude_reference!=AltitudeReference::AGL
            ||!std::isfinite(point.altitude_m)||point.altitude_m<0.0
            ||!std::isfinite(point.position.latitude_deg)
            ||!std::isfinite(point.position.longitude_deg)
            ||point.position.latitude_deg < -90.0 || point.position.latitude_deg > 90.0
            ||point.position.longitude_deg < -180.0 || point.position.longitude_deg > 180.0
            ||!points.emplace(point.waypoint_id,&point).second) {
            error="route waypoints require unique IDs, valid coordinates and explicit AGL altitudes";
            return false;
        }
    }
    std::vector<WindSample> samples;
    samples.reserve(route.segments.size());
    for (std::size_t i=0;i<route.segments.size();++i) {
        const auto& segment=route.segments[i];
        const auto from=points.find(segment.from_waypoint_id);
        const auto to=points.find(segment.to_waypoint_id);
        if (from==points.end()||to==points.end()) {
            error="route segment references an unknown waypoint: "+segment.segment_id;
            return false;
        }
        if (std::abs(to->second->position.longitude_deg-from->second->position.longitude_deg)>180.0) {
            error="route segment crosses the antimeridian, which this weather grid does not support";
            return false;
        }
        const double elapsed=offsets[i];
        if (!std::isfinite(elapsed)||elapsed<0.0
            ||elapsed>static_cast<double>(std::numeric_limits<std::int64_t>::max()-missionStart)) {
            error="route segment forecast sample time is invalid";
            return false;
        }
        const double latitude=(from->second->position.latitude_deg+to->second->position.latitude_deg)*0.5;
        const double longitude=from->second->position.longitude_deg
            +(to->second->position.longitude_deg-from->second->position.longitude_deg)*0.5;
        const double altitude=(from->second->altitude_m+to->second->altitude_m)*0.5;
        const auto sampleTime=missionStart+static_cast<std::int64_t>(std::llround(elapsed));
        const auto query=WindInterpolator::atTimeAndAltitude(
            fields,latitude,longitude,altitude,sampleTime,maximumVerticalGap,
            kMaximumForecastGapSeconds);
        if (!query.valid) {
            error="time-dependent wind sample rejected for segment "+segment.segment_id+": "+query.detail;
            return false;
        }
        samples.push_back({segment.segment_id,query.northward_mps,query.eastward_mps,0.0});
    }
    input.wind_samples=std::move(samples);
    return true;
}
} // namespace

WeatherTrajectoryEvaluation WeatherTrajectoryCalculator::evaluate(
    const bluesky::planning::Route& route,
    const std::vector<WindField>& fields,
    const std::string& snapshot_id,
    const std::string& snapshot_version,
    bluesky::planning::TrajectoryPerformanceInput input,
    const std::string& calculation_version,
    int maximum_vertical_gap_m)
{
    WeatherTrajectoryEvaluation result;
    if (!route.trajectory_geometry_complete) {
        result.weather_error="route trajectory geometry is incomplete; weather correction requires all flown legs and transition/turn segments";
        return result;
    }
    std::int64_t missionStart=0;
    if (!parseUtc(input.mission_start_utc,missionStart)) {
        result.weather_error="explicit valid UTC mission_start_utc is required for time-dependent weather";
        return result;
    }
    if (snapshot_id.empty()||snapshot_version.empty()||maximum_vertical_gap_m<=0) {
        result.weather_error="wind snapshot identity and vertical interpolation policy are required";
        return result;
    }
    std::vector<double> estimatedDurations;
    if (!initialDurations(route,input,estimatedDurations,result.weather_error)) return result;

    for (int iteration=0;iteration<kMaximumIterations;++iteration) {
        auto iterationInput=input;
        if (!buildTimeDependentWindSamples(route,fields,missionStart,
                midpointOffsets(estimatedDurations),maximum_vertical_gap_m,
                iterationInput,result.weather_error))
            return result;
        iterationInput.wind_snapshot_id=snapshot_id;
        iterationInput.wind_snapshot_version=snapshot_version;
        auto trajectory=bluesky::planning::WindPerformanceTrajectory::calculate(
            route,iterationInput,calculation_version);
        result.weather_ready=true;
        result.trajectory=std::move(trajectory);
        if (result.trajectory.status!=bluesky::planning::TrajectoryStatus::Feasible)
            return result;
        if (result.trajectory.segments.size()!=estimatedDurations.size()) {
            result.weather_ready=false;
            result.weather_error="trajectory result segment count changed during weather coupling";
            return result;
        }
        double maximumDelta=0.0;
        for (std::size_t i=0;i<estimatedDurations.size();++i) {
            const double actual=result.trajectory.segments[i].traversal_time_s;
            if (!std::isfinite(actual)||actual<=0.0) {
                result.weather_ready=false;
                result.weather_error="trajectory returned an invalid segment traversal time";
                return result;
            }
            maximumDelta=std::max(maximumDelta,std::abs(actual-estimatedDurations[i]));
        }
        if (maximumDelta<=kConvergenceToleranceSeconds) {
            result.weather_error.clear();
            return result;
        }
        for (std::size_t i=0;i<estimatedDurations.size();++i) {
            const double actual=result.trajectory.segments[i].traversal_time_s;
            estimatedDurations[i]=kDamping*estimatedDurations[i]+(1.0-kDamping)*actual;
        }
    }
    result.weather_ready=false;
    result.weather_error="time-dependent weather/trajectory iteration did not converge within 12 iterations";
    return result;
}

} // namespace bluesky::weather
