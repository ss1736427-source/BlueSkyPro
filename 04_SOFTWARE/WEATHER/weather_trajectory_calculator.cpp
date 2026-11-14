#include "weather_trajectory_calculator.hpp"
#include "route_wind_adapter.hpp"
#include "../PLANNING/camera_ground_speed_constraint.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <sstream>
#include <limits>
#include <unordered_map>
#include <utility>
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
    for (const auto& point:route.waypoints) {
        const bool hasDuration=point.task_execution_duration_s.has_value();
        const bool hasEnergy=point.task_execution_energy_wh.has_value();
        if (hasDuration!=hasEnergy
            || (hasDuration
                && (!std::isfinite(*point.task_execution_duration_s)
                    || *point.task_execution_duration_s<0.0
                    || !std::isfinite(*point.task_execution_energy_wh)
                    || *point.task_execution_energy_wh<0.0))) {
            error="task execution duration and energy must be supplied together as finite non-negative values";
            return false;
        }
        points.emplace(point.waypoint_id,&point);
    }
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

bool interpolateGeoPoint(const bluesky::planning::GeoPoint& a,
                        const bluesky::planning::GeoPoint& b,double fraction,
                        bluesky::planning::GeoPoint& result)
{
    constexpr double pi=3.14159265358979323846;
    const double lat1=a.latitude_deg*pi/180.0,lon1=a.longitude_deg*pi/180.0;
    const double lat2=b.latitude_deg*pi/180.0,lon2=b.longitude_deg*pi/180.0;
    const double ax=std::cos(lat1)*std::cos(lon1);
    const double ay=std::cos(lat1)*std::sin(lon1);
    const double az=std::sin(lat1);
    const double bx=std::cos(lat2)*std::cos(lon2);
    const double by=std::cos(lat2)*std::sin(lon2);
    const double bz=std::sin(lat2);
    const double dot=std::clamp(ax*bx+ay*by+az*bz,-1.0,1.0);
    const double angle=std::acos(dot);
    double x=0.0,y=0.0,z=0.0;
    if (angle<1e-10) {
        x=(1.0-fraction)*ax+fraction*bx;
        y=(1.0-fraction)*ay+fraction*by;
        z=(1.0-fraction)*az+fraction*bz;
    } else {
        const double sinAngle=std::sin(angle);
        if (std::abs(sinAngle)<1e-12) return false;
        const double wa=std::sin((1.0-fraction)*angle)/sinAngle;
        const double wb=std::sin(fraction*angle)/sinAngle;
        x=wa*ax+wb*bx;
        y=wa*ay+wb*by;
        z=wa*az+wb*bz;
    }
    const double norm=std::sqrt(x*x+y*y+z*z);
    if (!std::isfinite(norm)||norm<1e-12) return false;
    x/=norm; y/=norm; z/=norm;
    result.latitude_deg=std::asin(std::clamp(z,-1.0,1.0))*180.0/pi;
    result.longitude_deg=std::atan2(y,x)*180.0/pi;
    return std::isfinite(result.latitude_deg)&&std::isfinite(result.longitude_deg);
}

bool buildSamplingRoute(const bluesky::planning::Route& source,
                        bluesky::planning::TrajectoryPerformanceInput& input,
                        bluesky::planning::Route& sampled,
                        std::vector<std::size_t>& sourceSegmentIndices,
                        std::vector<double>& sourceOffsetsM,
                        std::vector<double>& sourceLengthsM,
                        std::string& error)
{
    using namespace bluesky::planning;
    // Bound samples to 10 km and 5 minutes so a long leg is evaluated along-track,
    // not represented by one midpoint against a 0.25-degree GFS grid.
    constexpr double kMaximumSampleDistanceM=10000.0;
    constexpr double kMaximumSampleDurationS=300.0;
    constexpr std::size_t kMaximumPiecesPerSegment=200;
    constexpr std::size_t kMaximumSampledSegments=5000;
    if (source.waypoints.size()<2 || source.segments.size()+1!=source.waypoints.size()) {
        error="complete ordered route segments are required for along-track weather sampling";
        return false;
    }
    std::vector<double> baseDurations;
    if (!initialDurations(source,input,baseDurations,error)) return false;
    auto originalCameraIds = input.camera_capture_segment_ids;
    std::sort(originalCameraIds.begin(), originalCameraIds.end());
    if (std::adjacent_find(originalCameraIds.begin(), originalCameraIds.end())
        != originalCameraIds.end()) {
        error = "camera capture segment identities must be unique before weather sampling";
        return false;
    }
    for (const auto& id : originalCameraIds) {
        if (std::none_of(source.segments.begin(), source.segments.end(),
                         [&](const auto& segment) { return segment.segment_id == id; })) {
            error = "camera capture segment identity is not present in the source route: " + id;
            return false;
        }
    }
    std::vector<std::string> sampledCameraIds;
    sourceSegmentIndices.clear();
    sourceOffsetsM.clear();
    sourceLengthsM.clear();
    sampled=source;
    sampled.waypoints.clear();
    sampled.segments.clear();
    sampled.speed_profile.reset();
    for (std::size_t i=0;i<source.segments.size();++i) {
        const auto& original=source.segments[i];
        const auto& from=source.waypoints[i];
        const auto& to=source.waypoints[i+1];
        if (original.from_waypoint_id!=from.waypoint_id
            || original.to_waypoint_id!=to.waypoint_id) {
            error="route segment chain is not ordered with canonical waypoints";
            return false;
        }
        if (std::abs(to.position.longitude_deg-from.position.longitude_deg)>180.0) {
            error="route segment crosses the antimeridian, which this weather grid does not support";
            return false;
        }
        const double distance=distanceMeters(from.position,to.position);
        const double time=baseDurations[i];
        const auto distancePieces=static_cast<std::size_t>(
            std::max(1.0,std::ceil(distance/kMaximumSampleDistanceM)));
        const auto timePieces=static_cast<std::size_t>(
            std::max(1.0,std::ceil(time/kMaximumSampleDurationS)));
        const std::size_t pieces=std::max(distancePieces,timePieces);
        if (pieces>kMaximumPiecesPerSegment
            || sampled.segments.size()+pieces>kMaximumSampledSegments) {
            error="route exceeds the configured along-track weather sampling density limit";
            return false;
        }
        if (sampled.waypoints.empty()) {
            sampled.waypoints.push_back(from);
        } else if (sampled.waypoints.back().waypoint_id!=from.waypoint_id) {
            error="canonical route waypoint chain is discontinuous";
            return false;
        }
        double sourceOffsetM=0.0;
        for (std::size_t piece=1;piece<=pieces;++piece) {
            RouteWaypoint next;
            if (piece==pieces) {
                next=to;
            } else {
                const double fraction=static_cast<double>(piece)/static_cast<double>(pieces);
                next=from;
                next.waypoint_id=from.waypoint_id+"__WX_WP__"+original.segment_id
                                 +"__"+std::to_string(piece);
                if (!interpolateGeoPoint(from.position,to.position,fraction,next.position)) {
                    error="cannot interpolate a stable geodesic point for weather sampling";
                    return false;
                }
                next.altitude_m=from.altitude_m+fraction*(to.altitude_m-from.altitude_m);
                next.mandatory=false;
            }
            RouteSegment subsegment;
            subsegment.segment_id=original.segment_id+"__WX_SEG__"+std::to_string(piece);
            subsegment.from_waypoint_id=sampled.waypoints.back().waypoint_id;
            subsegment.to_waypoint_id=next.waypoint_id;
            subsegment.distance_m=distanceMeters(sampled.waypoints.back().position,next.position);
            subsegment.nominal_speed_mps=original.nominal_speed_mps;
            subsegment.bank_angle_rad=original.bank_angle_rad;
            subsegment.lateral_acceleration_mps2=original.lateral_acceleration_mps2;
            if (std::binary_search(originalCameraIds.begin(), originalCameraIds.end(),
                                   original.segment_id)) {
                sampledCameraIds.push_back(subsegment.segment_id);
            }
            sourceSegmentIndices.push_back(i);
            sourceOffsetsM.push_back(sourceOffsetM);
            sourceLengthsM.push_back(subsegment.distance_m);
            sourceOffsetM+=subsegment.distance_m;
            sampled.segments.push_back(std::move(subsegment));
            sampled.waypoints.push_back(std::move(next));
        }
    }
    sampled.trajectory_geometry_complete=true;
    // Each weather subdivision inherits the camera-capture role of its source leg.
    input.camera_capture_segment_ids = std::move(sampledCameraIds);
    return sampled.segments.size()==sampled.waypoints.size()-1;
}

std::vector<double> midpointOffsets(
    const bluesky::planning::Route& route,
    const std::vector<double>& durations)
{
    std::vector<double> offsets;
    offsets.reserve(durations.size());
    if (route.waypoints.size() != durations.size() + 1)
        return offsets;
    double elapsed=0.0;
    for (std::size_t i=0;i<durations.size();++i) {
        const auto& departure=route.waypoints[i];
        if (departure.task_execution_duration_s.has_value())
            elapsed+=*departure.task_execution_duration_s;
        offsets.push_back(elapsed+durations[i]*0.5);
        elapsed+=durations[i];
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


double trackHeadingDegrees(const bluesky::planning::GeoPoint& from,
                           const bluesky::planning::GeoPoint& to)
{
    constexpr double pi=3.14159265358979323846;
    const double lat=(from.latitude_deg+to.latitude_deg)*0.5*pi/180.0;
    const double north=(to.latitude_deg-from.latitude_deg)*pi/180.0*6371000.0;
    const double east=(to.longitude_deg-from.longitude_deg)*pi/180.0
                     *std::cos(lat)*6371000.0;
    double heading=std::atan2(east,north)*180.0/pi;
    if (heading<0.0) heading+=360.0;
    return heading;
}

double profileSpeedAt(const bluesky::planning::SpeedProfileSegmentResult& p,
                      double distance)
{
    if (distance<p.acceleration_distance_m) {
        return std::sqrt(std::max(0.0,p.entry_speed_mps*p.entry_speed_mps
            +2.0*p.maximum_acceleration_mps2*distance));
    }
    const double brakingStart=p.distance_m-p.deceleration_distance_m;
    if (distance<=brakingStart) return p.maximum_reachable_speed_mps;
    const double remaining=std::max(0.0,p.distance_m-distance);
    return std::sqrt(std::max(0.0,p.exit_speed_mps*p.exit_speed_mps
        +2.0*p.maximum_deceleration_mps2*remaining));
}

bool buildWindAwareSpeedProfile(
    const bluesky::planning::Route& sourceRoute,
    bluesky::planning::Route& samplingRoute,
    const std::vector<std::size_t>& sourceSegmentIndices,
    const std::vector<double>& sourceOffsetsM,
    const std::vector<double>& sourceLengthsM,
    bluesky::planning::TrajectoryPerformanceInput& input,
    const std::string& snapshotId,
    const std::string& snapshotVersion,
    const std::string& calculationVersion,
    bluesky::planning::Route& correctedRoute,
    std::vector<double>& previousProfilePeaks,
    double& maximumProfileDelta,
    std::string& error)
{
    using namespace bluesky::planning;
    const bool cameraConfigured=!input.camera_configuration_id.empty()
        ||input.camera_gsd_m_px!=0.0||input.camera_forward_trigger_spacing_m!=0.0
        ||input.camera_minimum_trigger_interval_s!=0.0||input.camera_exposure_time_s!=0.0
        ||input.camera_maximum_motion_blur_px!=0.0||!input.camera_capture_segment_ids.empty();
    if (!sourceRoute.speed_profile) {
        if (cameraConfigured) {
            error="camera ground-speed feedback requires a canonical route with a validated longitudinal speed envelope";
            return false;
        }
        if (!input.speed_profile_segments.empty()) {
            error="external speed profile cannot be safely remapped to weather subdivisions without canonical route speed-envelope metadata";
            return false;
        }
        maximumProfileDelta=0.0;
        return true;
    }

    const auto& sourceProfile=*sourceRoute.speed_profile;
    if (sourceProfile.vehicle_model_id.empty()||sourceProfile.vehicle_model_version.empty()
        ||sourceProfile.configuration_id.empty()||sourceProfile.dynamics_source.empty()
        ||(sourceProfile.speed_reference!="TAS"&&sourceProfile.speed_reference!="AIR_RELATIVE")
        ||sourceProfile.segments.size()!=sourceRoute.segments.size()
        ||sourceProfile.segments.empty()
        ||sourceSegmentIndices.size()!=samplingRoute.segments.size()
        ||sourceOffsetsM.size()!=samplingRoute.segments.size()
        ||sourceLengthsM.size()!=samplingRoute.segments.size()) {
        error="wind/camera speed feedback requires a complete, identified source speed profile and segment mapping";
        return false;
    }

    const auto& first=sourceProfile.segments.front();
    const auto& last=sourceProfile.segments.back();
    if (!std::isfinite(first.maximum_acceleration_mps2)||first.maximum_acceleration_mps2<=0.0
        ||!std::isfinite(first.maximum_deceleration_mps2)||first.maximum_deceleration_mps2<=0.0
        ||!std::isfinite(first.entry_speed_mps)||first.entry_speed_mps<=0.0
        ||!std::isfinite(last.exit_speed_mps)||last.exit_speed_mps<=0.0) {
        error="wind/camera speed feedback is missing validated acceleration, deceleration or route boundary speeds";
        return false;
    }

    SpeedProfileInput profileInput;
    profileInput.vehicle_model_id=sourceProfile.vehicle_model_id;
    profileInput.vehicle_model_version=sourceProfile.vehicle_model_version;
    profileInput.configuration_id=sourceProfile.configuration_id;
    profileInput.speed_reference=sourceProfile.speed_reference;
    profileInput.longitudinal_dynamics_validated=true;
    profileInput.longitudinal_dynamics_source=sourceProfile.dynamics_source;
    profileInput.maximum_acceleration_mps2=first.maximum_acceleration_mps2;
    profileInput.maximum_deceleration_mps2=first.maximum_deceleration_mps2;
    profileInput.initial_speed_mps=first.entry_speed_mps;
    profileInput.final_speed_mps=last.exit_speed_mps;
    profileInput.segments.reserve(sourceRoute.segments.size());
    std::vector<double> sourceSampleDistances(sourceRoute.segments.size(),0.0);
    for (std::size_t i=0;i<sourceLengthsM.size();++i) {
        if (sourceSegmentIndices[i]>=sourceSampleDistances.size()) {
            error="weather subdivision references an unknown source segment";
            return false;
        }
        sourceSampleDistances[sourceSegmentIndices[i]]+=sourceLengthsM[i];
    }

    for (std::size_t i=0;i<sourceRoute.segments.size();++i) {
        const auto& routeSegment=sourceRoute.segments[i];
        const auto& profileSegment=sourceProfile.segments[i];
        if (profileSegment.segment_id!=routeSegment.segment_id
            ||!std::isfinite(profileSegment.minimum_speed_mps)||profileSegment.minimum_speed_mps<=0.0
            ||!std::isfinite(profileSegment.maximum_speed_mps)
            ||profileSegment.maximum_speed_mps<profileSegment.minimum_speed_mps
            ||!std::isfinite(sourceSampleDistances[i])||sourceSampleDistances[i]<=0.0
            ||std::abs(sourceSampleDistances[i]-routeSegment.distance_m)
                >std::max(0.5,routeSegment.distance_m*0.01)
            ||std::abs(profileSegment.distance_m-routeSegment.distance_m)
                >std::max(0.5,routeSegment.distance_m*0.01)
            ||std::abs(profileSegment.maximum_acceleration_mps2-profileInput.maximum_acceleration_mps2)>1e-6
            ||std::abs(profileSegment.maximum_deceleration_mps2-profileInput.maximum_deceleration_mps2)>1e-6) {
            error="canonical route speed envelope is incomplete, inconsistent or not bound to exact segment identities";
            return false;
        }
        profileInput.segments.push_back({routeSegment.segment_id,sourceSampleDistances[i],
            profileSegment.minimum_speed_mps,profileSegment.maximum_speed_mps});
    }

    std::unordered_map<std::string,const WindSample*> windBySegment;
    for (const auto& wind:input.wind_samples) windBySegment.emplace(wind.segment_id,&wind);
    const double triggerCap=input.camera_minimum_trigger_interval_s>0.0
        ?input.camera_forward_trigger_spacing_m/input.camera_minimum_trigger_interval_s
        :std::numeric_limits<double>::infinity();
    const double blurCap=input.camera_exposure_time_s>0.0
        ?input.camera_maximum_motion_blur_px*input.camera_gsd_m_px/input.camera_exposure_time_s
        :std::numeric_limits<double>::infinity();
    const double cameraGroundSpeedCap=std::min(triggerCap,blurCap);
    if (cameraConfigured && (!std::isfinite(cameraGroundSpeedCap)||cameraGroundSpeedCap<=0.0
        ||!std::isfinite(input.camera_gsd_m_px)||input.camera_gsd_m_px<=0.0
        ||!std::isfinite(input.camera_forward_trigger_spacing_m)||input.camera_forward_trigger_spacing_m<=0.0
        ||!std::isfinite(input.camera_minimum_trigger_interval_s)||input.camera_minimum_trigger_interval_s<=0.0
        ||!std::isfinite(input.camera_exposure_time_s)||input.camera_exposure_time_s<=0.0
        ||!std::isfinite(input.camera_maximum_motion_blur_px)||input.camera_maximum_motion_blur_px<=0.0
        ||input.camera_capture_segment_ids.empty())) {
        error="camera trigger/blur parameters are incomplete for a ground-speed envelope";
        return false;
    }

    for (std::size_t i=0;i<samplingRoute.segments.size();++i) {
        const auto& sampledSegment=samplingRoute.segments[i];
        if (!cameraConfigured
            ||std::find(input.camera_capture_segment_ids.begin(),input.camera_capture_segment_ids.end(),
                        sampledSegment.segment_id)==input.camera_capture_segment_ids.end())
            continue;
        const auto sourceIndex=sourceSegmentIndices[i];
        if (sourceIndex>=profileInput.segments.size()) {
            error="sampled weather segment has no source route segment";
            return false;
        }
        const auto windIt=windBySegment.find(sampledSegment.segment_id);
        if (windIt==windBySegment.end()) {
            error="camera speed feedback has no time-dependent wind sample for segment "+sampledSegment.segment_id;
            return false;
        }
        const auto& from=samplingRoute.waypoints[i];
        const auto& to=samplingRoute.waypoints[i+1];
        auto& bounds=profileInput.segments[sourceIndex];
        const auto cap=CameraGroundSpeedConstraint::deriveAirspeedCap(
            trackHeadingDegrees(from.position,to.position),*windIt->second,
            cameraGroundSpeedCap,bounds.minimum_speed_mps,bounds.maximum_speed_mps);
        auto sourceBoundCap=cap;
        sourceBoundCap.segment_id=bounds.segment_id;
        if (!cap.feasible||!CameraGroundSpeedConstraint::applyToSegment(sourceBoundCap,bounds)) {
            error="camera ground-speed envelope is infeasible on source segment "
                +sourceRoute.segments[sourceIndex].segment_id+": "+cap.diagnostic;
            return false;
        }
    }

    const auto solved=VariableSpeedProfileSolver::calculate(profileInput);
    if (solved.status!=SpeedProfileStatus::Feasible
        ||solved.segments.size()!=sourceRoute.segments.size()) {
        error=solved.findings.empty()?"wind/camera constrained speed profile is infeasible":solved.findings.front();
        return false;
    }

    if (previousProfilePeaks.empty()) {
        previousProfilePeaks.reserve(samplingRoute.segments.size());
        for (const auto sourceIndex:sourceSegmentIndices)
            previousProfilePeaks.push_back(sourceProfile.segments[sourceIndex].maximum_reachable_speed_mps);
    }
    if (previousProfilePeaks.size()!=samplingRoute.segments.size()) {
        error="weather profile convergence state does not match sampled route segmentation";
        return false;
    }

    std::vector<SpeedProfileSegmentResult> sampledProfile;
    sampledProfile.reserve(samplingRoute.segments.size());
    maximumProfileDelta=0.0;
    for (std::size_t i=0;i<samplingRoute.segments.size();++i) {
        const auto sourceIndex=sourceSegmentIndices[i];
        const auto& original=solved.segments[sourceIndex];
        const double totalSampleDistance=sourceSampleDistances[sourceIndex];
        if (totalSampleDistance<=0.0) {
            error="weather subdivision produced an empty source segment";
            return false;
        }
        const double scale=original.distance_m/totalSampleDistance;
        const double start=sourceOffsetsM[i]*scale;
        const double length=sourceLengthsM[i]*scale;
        const double end=start+length;
        const double entry=profileSpeedAt(original,start);
        const double exit=profileSpeedAt(original,end);
        const double brakingStart=original.distance_m-original.deceleration_distance_m;
        const double accelDistance=std::max(0.0,std::min(end,original.acceleration_distance_m)-start);
        const double decelDistance=std::max(0.0,end-std::max(start,brakingStart));
        const double cruiseDistance=std::max(0.0,length-accelDistance-decelDistance);
        const bool intersectsPeak=(start<=original.acceleration_distance_m+1e-8
            &&end>=original.acceleration_distance_m-1e-8)
            ||(start<=brakingStart+1e-8&&end>=brakingStart-1e-8)
            ||(start>=original.acceleration_distance_m-1e-8&&end<=brakingStart+1e-8);
        const double peak=intersectsPeak?original.maximum_reachable_speed_mps:std::max(entry,exit);
        const double actualLength=samplingRoute.segments[i].distance_m;
        const double actualScale=actualLength/length;
        sampledProfile.push_back({samplingRoute.segments[i].segment_id,actualLength,entry,exit,peak,
            accelDistance*actualScale,decelDistance*actualScale,cruiseDistance*actualScale,
            original.maximum_acceleration_mps2,original.maximum_deceleration_mps2});
        maximumProfileDelta=std::max(maximumProfileDelta,std::abs(peak-previousProfilePeaks[i]));
        previousProfilePeaks[i]=peak;
    }

    RouteSpeedProfile updated=sourceProfile;
    updated.profile_id=sourceRoute.lineage.route_id+"-"+sourceRoute.lineage.route_version+"-WIND-SPEED";
    std::ostringstream profileVersion;
    profileVersion<<sourceRoute.lineage.mission_id<<'|'<<sourceRoute.lineage.mission_version
        <<'|'<<sourceRoute.lineage.route_id<<'|'<<sourceRoute.lineage.route_version
        <<'|'<<sourceRoute.lineage.calculation_input_version
        <<'|'<<sourceProfile.vehicle_model_id<<'|'<<sourceProfile.vehicle_model_version
        <<'|'<<sourceProfile.configuration_id<<'|'<<sourceProfile.speed_reference
        <<'|'<<sourceProfile.dynamics_source
        <<'|'<<snapshotId<<'|'<<snapshotVersion<<'|'<<calculationVersion
        <<'|'<<input.camera_configuration_id<<'|'<<std::setprecision(17)
        <<input.camera_gsd_m_px<<'|'<<input.camera_forward_trigger_spacing_m
        <<'|'<<input.camera_minimum_trigger_interval_s<<'|'<<input.camera_exposure_time_s
        <<'|'<<input.camera_maximum_motion_blur_px;
    updated.profile_version=profileVersion.str();
    updated.wind_snapshot_id=snapshotId;
    updated.wind_snapshot_version=snapshotVersion;
    updated.camera_configuration_id=input.camera_configuration_id;
    input.speed_profile_id=updated.profile_id;
    input.speed_profile_version=updated.profile_version;
    input.speed_profile_vehicle_model_id=sourceProfile.vehicle_model_id;
    input.speed_profile_vehicle_model_version=sourceProfile.vehicle_model_version;
    input.speed_profile_configuration_id=sourceProfile.configuration_id;
    input.speed_profile_speed_reference=sourceProfile.speed_reference;
    input.speed_profile_segments=sampledProfile;

    updated.segments.clear();
    updated.segments.reserve(solved.segments.size());
    for (std::size_t i=0;i<solved.segments.size();++i) {
        const auto& p=solved.segments[i];
        const auto& bounds=profileInput.segments[i];
        updated.segments.push_back({p.segment_id,p.distance_m,p.entry_speed_mps,p.exit_speed_mps,
            p.maximum_reachable_speed_mps,p.acceleration_distance_m,p.deceleration_distance_m,
            p.cruise_distance_m,p.maximum_acceleration_mps2,p.maximum_deceleration_mps2,
            sourceProfile.segments[i].minimum_speed_mps,
            sourceProfile.segments[i].maximum_speed_mps,
            bounds.minimum_speed_mps,bounds.maximum_speed_mps});
    }
    correctedRoute=sourceRoute;
    correctedRoute.environment.wind_snapshot_id=snapshotId;
    correctedRoute.speed_profile=updated;
    samplingRoute.speed_profile=updated;
    samplingRoute.speed_profile->segments.clear();
    samplingRoute.speed_profile->segments.reserve(sampledProfile.size());
    for (std::size_t i=0;i<sampledProfile.size();++i) {
        const auto& p=sampledProfile[i];
        const auto& bounds=profileInput.segments[sourceSegmentIndices[i]];
        samplingRoute.speed_profile->segments.push_back({p.segment_id,p.distance_m,
            p.entry_speed_mps,p.exit_speed_mps,p.maximum_reachable_speed_mps,
            p.acceleration_distance_m,p.deceleration_distance_m,p.cruise_distance_m,
            p.maximum_acceleration_mps2,p.maximum_deceleration_mps2,
            sourceProfile.segments[sourceSegmentIndices[i]].minimum_speed_mps,
            sourceProfile.segments[sourceSegmentIndices[i]].maximum_speed_mps,
            bounds.minimum_speed_mps,bounds.maximum_speed_mps});
    }
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
    bluesky::planning::Route samplingRoute;
    std::vector<std::size_t> sourceSegmentIndices;
    std::vector<double> sourceOffsetsM,sourceLengthsM;
    if (!buildSamplingRoute(route,input,samplingRoute,sourceSegmentIndices,
            sourceOffsetsM,sourceLengthsM,result.weather_error)) return result;
    std::vector<double> estimatedDurations;
    if (!initialDurations(samplingRoute,input,estimatedDurations,result.weather_error)) return result;

    bluesky::planning::Route correctedRoute=route;
    bool profileApplied=false;
    std::vector<double> previousProfilePeaks;
    for (int iteration=0;iteration<kMaximumIterations;++iteration) {
        auto iterationInput=input;
        if (!buildTimeDependentWindSamples(samplingRoute,fields,missionStart,
                midpointOffsets(samplingRoute,estimatedDurations),maximum_vertical_gap_m,
                iterationInput,result.weather_error))
            return result;
        iterationInput.wind_snapshot_id=snapshot_id;
        iterationInput.wind_snapshot_version=snapshot_version;
        iterationInput.mission_start_epoch_s=missionStart;
        double maximumProfileDelta=0.0;
        if (route.speed_profile) {
            if (!buildWindAwareSpeedProfile(route,samplingRoute,sourceSegmentIndices,
                    sourceOffsetsM,sourceLengthsM,iterationInput,snapshot_id,snapshot_version,
                    calculation_version,correctedRoute,previousProfilePeaks,
                    maximumProfileDelta,result.weather_error))
                return result;
            profileApplied=true;
        } else if (!iterationInput.camera_capture_segment_ids.empty()) {
            result.weather_error="camera speed feedback is blocked: no validated route speed profile is attached";
            return result;
        }
        auto trajectory=bluesky::planning::WindPerformanceTrajectory::calculate(
            samplingRoute,iterationInput,calculation_version);
        result.trajectory=std::move(trajectory);
        if (result.trajectory.status!=bluesky::planning::TrajectoryStatus::Feasible) {
            if (!result.trajectory.findings.empty())
                result.weather_error = result.trajectory.findings.front().detail;
            return result;
        }
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
        if (maximumDelta<=kConvergenceToleranceSeconds&&maximumProfileDelta<=0.05) {
            result.weather_ready=true;
            result.weather_error.clear();
            if (profileApplied) result.corrected_route=correctedRoute;
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
