#include "wind_performance_trajectory.hpp"
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>

namespace bluesky::planning {
namespace {
constexpr double kPi=3.14159265358979323846;
constexpr double kEarthRadiusM=6371000.0;

double deg_to_rad(double v){ return v*kPi/180.0; }
double rad_to_deg(double v){ return v*180.0/kPi; }

double distance_m(const GeoPoint&a,const GeoPoint&b){
    const double lat1=deg_to_rad(a.latitude_deg), lat2=deg_to_rad(b.latitude_deg);
    const double dlat=lat2-lat1, dlon=deg_to_rad(b.longitude_deg-a.longitude_deg);
    const double x=dlon*std::cos((lat1+lat2)*0.5)*kEarthRadiusM;
    const double y=dlat*kEarthRadiusM;
    return std::hypot(x,y);
}

void local_vector(const GeoPoint&a,const GeoPoint&b,double& north,double& east){
    const double lat=deg_to_rad((a.latitude_deg+b.latitude_deg)*0.5);
    north=deg_to_rad(b.latitude_deg-a.latitude_deg)*kEarthRadiusM;
    east=deg_to_rad(b.longitude_deg-a.longitude_deg)*kEarthRadiusM*std::cos(lat);
}

double heading_deg(double north,double east){
    double h=rad_to_deg(std::atan2(east,north));
    if(h<0.0) h+=360.0;
    return h;
}

const WindSample* find_wind(const std::vector<WindSample>& samples,const std::string&id){
    for(const auto& s:samples) if(s.segment_id==id) return &s;
    return nullptr;
}

const SpeedProfileSegmentResult* find_profile_segment(
    const std::vector<SpeedProfileSegmentResult>& segments, const std::string& id) {
    for (const auto& segment : segments) if (segment.segment_id == id) return &segment;
    return nullptr;
}

double profile_airspeed_at(const SpeedProfileSegmentResult& profile, double distance_m) {
    if (distance_m < profile.acceleration_distance_m) {
        return std::sqrt(std::max(0.0, profile.entry_speed_mps * profile.entry_speed_mps
            + 2.0 * profile.maximum_acceleration_mps2 * distance_m));
    }
    const double brakingStart = profile.distance_m - profile.deceleration_distance_m;
    if (distance_m <= brakingStart) return profile.maximum_reachable_speed_mps;
    const double remaining = std::max(0.0, profile.distance_m - distance_m);
    return std::sqrt(std::max(0.0, profile.exit_speed_mps * profile.exit_speed_mps
        + 2.0 * profile.maximum_deceleration_mps2 * remaining));
}

std::string dependency(const Route&r,const TrajectoryPerformanceInput&i,const std::string&v){
    std::ostringstream o;
    o<<r.lineage.mission_id<<'|'<<r.lineage.mission_version
     <<'|'<<r.lineage.route_id<<'|'<<r.lineage.route_version
     <<'|'<<r.lineage.calculation_input_version
     <<'|'<<i.uav_id<<'|'<<i.configuration_version<<'|'<<i.performance_version
     <<'|'<<i.wind_snapshot_id<<'|'<<i.wind_snapshot_version<<'|'<<i.mission_start_utc
     <<'|'<<i.mission_start_epoch_s<<'|'<<v
     <<'|'<<i.speed_profile_id<<'|'<<i.speed_profile_version<<'|'<<i.speed_profile_vehicle_model_id
     <<'|'<<i.speed_profile_vehicle_model_version<<'|'<<i.speed_profile_configuration_id
     <<'|'<<i.speed_profile_speed_reference
     <<'|'<<std::setprecision(17)
     <<i.cruise_airspeed_mps<<'|'<<i.climb_rate_mps<<'|'<<i.descent_rate_mps
     <<'|'<<i.wind_tolerance_mps<<'|'<<i.energy_per_horizontal_meter_wh
     <<'|'<<i.energy_per_climb_meter_wh<<'|'<<i.energy_per_descent_meter_wh
     <<'|'<<i.payload_power_w<<'|'<<i.camera_configuration_id
     <<'|'<<i.camera_gsd_m_px<<'|'<<i.camera_forward_trigger_spacing_m
     <<'|'<<i.camera_minimum_trigger_interval_s<<'|'<<i.camera_exposure_time_s
     <<'|'<<i.camera_maximum_motion_blur_px<<'|'<<i.usable_energy_wh<<'|'<<i.reserve_requirement_wh
     <<'|'<<r.trajectory_geometry_complete;
    for (const auto& point : r.waypoints) {
        o << "|WP:" << point.waypoint_id << ':' << std::setprecision(17)
          << point.position.latitude_deg << ',' << point.position.longitude_deg
          << ',' << point.altitude_m << ','
          << static_cast<int>(point.altitude_reference);
        if (point.task_execution_duration_s.has_value())
            o << ",TASK_TIME:" << *point.task_execution_duration_s;
        if (point.task_execution_energy_wh.has_value())
            o << ",TASK_ENERGY:" << *point.task_execution_energy_wh;
    }
    for (const auto& segment : r.segments)
        o << "|SEG:" << segment.segment_id << ':' << segment.from_waypoint_id
          << '>' << segment.to_waypoint_id << ':' << segment.distance_m
          << ',' << segment.nominal_speed_mps;
    for (const auto& profile : i.speed_profile_segments)
        o << "|SPEED_PROFILE:" << profile.segment_id << ':' << std::setprecision(17)
          << profile.distance_m << ',' << profile.entry_speed_mps << ',' << profile.exit_speed_mps
          << ',' << profile.maximum_reachable_speed_mps << ',' << profile.acceleration_distance_m
          << ',' << profile.deceleration_distance_m << ',' << profile.cruise_distance_m
          << ',' << profile.maximum_acceleration_mps2 << ',' << profile.maximum_deceleration_mps2;
    for (const auto& id : i.camera_capture_segment_ids) o << "|CAMERA_SEG:" << id;
    for(const auto&s:i.wind_samples)
        o<<'|'<<s.segment_id<<':'<<std::setprecision(17)<<s.north_mps<<','<<s.east_mps<<','<<s.up_mps;
    return o.str();
}

void fail(TrajectoryResult&r,TrajectoryFindingCode c,const std::string&sid,const std::string&d){
    r.status=TrajectoryStatus::Infeasible;
    r.findings.push_back({c,sid,d});
}
}

TrajectoryResult WindPerformanceTrajectory::calculate(
    const Route& route,const TrajectoryPerformanceInput& input,const std::string&calculation_version){
    TrajectoryResult r;
    r.route_id=route.lineage.route_id;
    r.route_version=route.lineage.route_version;
    r.mission_start_epoch_s=input.mission_start_epoch_s;
    r.uav_id=input.uav_id;
    r.configuration_version=input.configuration_version;
    r.performance_version=input.performance_version;
    r.wind_snapshot_id=input.wind_snapshot_id;
    r.wind_snapshot_version=input.wind_snapshot_version;
    r.calculation_version=calculation_version;
    r.usable_energy_wh=input.usable_energy_wh;
    r.reserve_requirement_wh=input.reserve_requirement_wh;
    r.dependency_identity=dependency(route,input,calculation_version);

    const bool variableProfile = !input.speed_profile_segments.empty();
    if((!variableProfile && input.cruise_airspeed_mps<=0.0)
       || input.climb_rate_mps<=0.0 ||
       input.descent_rate_mps<=0.0 || input.wind_tolerance_mps<0.0 ||
       input.energy_per_horizontal_meter_wh<0.0 ||
       input.energy_per_climb_meter_wh<0.0 ||
       input.energy_per_descent_meter_wh<0.0 ||
       !std::isfinite(input.payload_power_w) || input.payload_power_w<0.0 ||
       input.usable_energy_wh<0.0 || input.reserve_requirement_wh<0.0 ||
       input.mission_start_epoch_s<0){
        fail(r,TrajectoryFindingCode::InvalidPerformance,"","performance inputs must be non-negative and speeds/rates positive");
        return r;
    }

    const bool cameraConfigured = !input.camera_configuration_id.empty()
        || input.camera_gsd_m_px != 0.0 || input.camera_forward_trigger_spacing_m != 0.0
        || input.camera_minimum_trigger_interval_s != 0.0 || input.camera_exposure_time_s != 0.0
        || input.camera_maximum_motion_blur_px != 0.0 || !input.camera_capture_segment_ids.empty();
    if (cameraConfigured) {
        const bool validCamera = !input.camera_configuration_id.empty()
            && std::isfinite(input.camera_gsd_m_px) && input.camera_gsd_m_px > 0.0
            && std::isfinite(input.camera_forward_trigger_spacing_m) && input.camera_forward_trigger_spacing_m > 0.0
            && std::isfinite(input.camera_minimum_trigger_interval_s) && input.camera_minimum_trigger_interval_s > 0.0
            && std::isfinite(input.camera_exposure_time_s) && input.camera_exposure_time_s > 0.0
            && std::isfinite(input.camera_maximum_motion_blur_px) && input.camera_maximum_motion_blur_px > 0.0
            && !input.camera_capture_segment_ids.empty();
        if (!validCamera) {
            fail(r, TrajectoryFindingCode::CameraCaptureConfigurationIncomplete, "",
                 "camera configuration, GSD, trigger spacing, minimum interval, exposure, blur tolerance and coverage segment identities are all required together");
            return r;
        }
        auto ids = input.camera_capture_segment_ids;
        std::sort(ids.begin(), ids.end());
        if (std::adjacent_find(ids.begin(), ids.end()) != ids.end()) {
            fail(r, TrajectoryFindingCode::CameraCaptureConfigurationIncomplete, "",
                 "camera coverage segment identities must be unique");
            return r;
        }
        for (const auto& id : ids) {
            if (std::none_of(route.segments.begin(), route.segments.end(), [&](const auto& segment) { return segment.segment_id == id; })) {
                fail(r, TrajectoryFindingCode::CameraCaptureConfigurationIncomplete, id,
                     "camera coverage segment identity does not exist in the canonical route");
                return r;
            }
        }
    }

    if(route.waypoints.empty() || route.segments.size()+1!=route.waypoints.size()){
        fail(r,TrajectoryFindingCode::InvalidPerformance,"","route waypoint/segment structure is invalid");
        return r;
    }

    if (variableProfile) {
        if (input.speed_profile_id.empty() || input.speed_profile_version.empty()
            || input.speed_profile_vehicle_model_id.empty()
            || input.speed_profile_vehicle_model_version.empty()
            || input.speed_profile_configuration_id.empty()
            || (input.speed_profile_speed_reference != "TAS"
                && input.speed_profile_speed_reference != "AIR_RELATIVE")
            || input.speed_profile_segments.size() != route.segments.size()) {
            fail(r, TrajectoryFindingCode::InvalidPerformance, "",
                 "variable speed profile identity, air-relative reference and complete route coverage are mandatory");
            return r;
        }
        for (std::size_t i = 0; i < route.segments.size(); ++i) {
            const auto& routeSegment = route.segments[i];
            const auto& profile = input.speed_profile_segments[i];
            if (profile.segment_id != routeSegment.segment_id
                || !std::isfinite(profile.distance_m) || profile.distance_m <= 0.0
                || !std::isfinite(profile.entry_speed_mps) || profile.entry_speed_mps <= 0.0
                || !std::isfinite(profile.exit_speed_mps) || profile.exit_speed_mps <= 0.0
                || !std::isfinite(profile.maximum_reachable_speed_mps)
                || profile.maximum_reachable_speed_mps <= 0.0
                || !std::isfinite(profile.acceleration_distance_m) || profile.acceleration_distance_m < 0.0
                || !std::isfinite(profile.deceleration_distance_m) || profile.deceleration_distance_m < 0.0
                || !std::isfinite(profile.cruise_distance_m) || profile.cruise_distance_m < 0.0
                || !std::isfinite(profile.maximum_acceleration_mps2) || profile.maximum_acceleration_mps2 <= 0.0
                || !std::isfinite(profile.maximum_deceleration_mps2) || profile.maximum_deceleration_mps2 <= 0.0
                || profile.maximum_reachable_speed_mps + 1e-9
                    < std::max(profile.entry_speed_mps, profile.exit_speed_mps)
                || std::abs(profile.distance_m - routeSegment.distance_m) > std::max(0.5, routeSegment.distance_m * 0.01)
                || std::abs(profile.distance_m - profile.acceleration_distance_m
                    - profile.deceleration_distance_m - profile.cruise_distance_m) > 1e-5
                || std::abs(profile.acceleration_distance_m
                    - std::max(0.0, (profile.maximum_reachable_speed_mps * profile.maximum_reachable_speed_mps
                        - profile.entry_speed_mps * profile.entry_speed_mps)
                        / (2.0 * profile.maximum_acceleration_mps2))) > std::max(1e-4, profile.distance_m * 1e-6)
                || std::abs(profile.deceleration_distance_m
                    - std::max(0.0, (profile.maximum_reachable_speed_mps * profile.maximum_reachable_speed_mps
                        - profile.exit_speed_mps * profile.exit_speed_mps)
                        / (2.0 * profile.maximum_deceleration_mps2))) > std::max(1e-4, profile.distance_m * 1e-6)
                || (i > 0 && std::abs(profile.entry_speed_mps
                    - input.speed_profile_segments[i - 1].exit_speed_mps) > 1e-6)) {
                fail(r, TrajectoryFindingCode::InvalidPerformance, routeSegment.segment_id,
                     "variable speed profile segment does not match canonical route geometry or contains invalid dynamics");
                return r;
            }
        }
    }

    for (const auto& waypoint : route.waypoints) {
        const bool hasDuration = waypoint.task_execution_duration_s.has_value();
        const bool hasEnergy = waypoint.task_execution_energy_wh.has_value();
        if (hasDuration != hasEnergy
            || (hasDuration && (!std::isfinite(*waypoint.task_execution_duration_s)
                                || *waypoint.task_execution_duration_s < 0.0
                                || !std::isfinite(*waypoint.task_execution_energy_wh)
                                || *waypoint.task_execution_energy_wh < 0.0))) {
            fail(r, TrajectoryFindingCode::InvalidPerformance, waypoint.waypoint_id,
                 "task execution duration and energy must be supplied together as finite non-negative values");
            return r;
        }
    }

    r.points.push_back({route.waypoints.front().waypoint_id,route.waypoints.front().position,
                        route.waypoints.front().altitude_m,0.0,0.0});

    double time=0.0, energy=0.0;
    for(const auto&seg:route.segments){
        const RouteWaypoint* from=nullptr; const RouteWaypoint* to=nullptr;
        for(const auto&w:route.waypoints){
            if(w.waypoint_id==seg.from_waypoint_id) from=&w;
            if(w.waypoint_id==seg.to_waypoint_id) to=&w;
        }
        if(!from || !to){
            fail(r,TrajectoryFindingCode::InvalidPerformance,seg.segment_id,"segment waypoint reference is missing");
            return r;
        }

        // Account for the task executed at the departure waypoint before
        // time-indexing the next flight segment's weather.
        if (from->task_execution_duration_s.has_value()) {
            time += *from->task_execution_duration_s;
            energy += *from->task_execution_energy_wh;
            r.points.back().elapsed_time_s = time;
            r.points.back().cumulative_energy_wh = energy;
            if (energy + input.reserve_requirement_wh > input.usable_energy_wh) {
                fail(r, TrajectoryFindingCode::InsufficientEnergyReserve, from->waypoint_id,
                     "task execution exceeds usable energy after reserve");
                return r;
            }
        }

        const WindSample* wind=find_wind(input.wind_samples,seg.segment_id);
        if(!wind){
            fail(r,TrajectoryFindingCode::MissingWindSample,seg.segment_id,"wind sample is required for every route segment");
            return r;
        }

        const double wind_speed=std::hypot(wind->north_mps,wind->east_mps);
        if(!std::isfinite(wind_speed) || !std::isfinite(wind->up_mps)){
            fail(r,TrajectoryFindingCode::InvalidWindSample,seg.segment_id,"wind sample is not finite");
            return r;
        }
        if(wind_speed>input.wind_tolerance_mps){
            fail(r,TrajectoryFindingCode::WindExceedsTolerance,seg.segment_id,"wind exceeds UAV tolerance");
            return r;
        }

        double north=0.0,east=0.0;
        local_vector(from->position,to->position,north,east);
        const double horizontal=std::hypot(north,east);
        if (segmentProfile && std::abs(segmentProfile->distance_m - horizontal)
            > std::max(0.5, horizontal * 0.01)) {
            fail(r, TrajectoryFindingCode::InvalidPerformance, seg.segment_id,
                 "speed profile distance does not match the canonical waypoint geometry");
            return r;
        }
        const double dz=to->altitude_m-from->altitude_m;
        const auto* segmentProfile = variableProfile
            ? find_profile_segment(input.speed_profile_segments, seg.segment_id) : nullptr;
        const double fallbackAirspeed = variableProfile
            ? segmentProfile->maximum_reachable_speed_mps : input.cruise_airspeed_mps;
        const double vertical_rate=dz>0.0?input.climb_rate_mps:(dz<0.0?input.descent_rate_mps:fallbackAirspeed);

        double duration=0.0, ground_speed=0.0, maximum_ground_speed=0.0, track=heading_deg(north,east);
        if(horizontal>1e-9){
            const double hn=north/horizontal, he=east/horizontal;
            if (segmentProfile) {
                // Deterministic distance-domain integration of the validated
                // acceleration / cruise / braking phases under this segment's wind.
                constexpr int kIntegrationSteps = 64;
                double northDisplacement = 0.0, eastDisplacement = 0.0;
                const auto groundSpeedAt = [&](double airspeed) {
                    return std::hypot(airspeed * hn + wind->north_mps,
                                      airspeed * he + wind->east_mps);
                };
                maximum_ground_speed = std::max({
                    groundSpeedAt(segmentProfile->entry_speed_mps),
                    groundSpeedAt(segmentProfile->maximum_reachable_speed_mps),
                    groundSpeedAt(segmentProfile->exit_speed_mps)});
                for (int step = 0; step < kIntegrationSteps; ++step) {
                    const double ds = horizontal / static_cast<double>(kIntegrationSteps);
                    const double alongDistance = (static_cast<double>(step) + 0.5)
                        * segmentProfile->distance_m / static_cast<double>(kIntegrationSteps);
                    const double airspeed = profile_airspeed_at(*segmentProfile, alongDistance);
                    const double an = airspeed * hn, ae = airspeed * he;
                    const double gn = an + wind->north_mps, ge = ae + wind->east_mps;
                    const double gs = std::hypot(gn, ge);
                    if (!std::isfinite(gs) || gs <= 1e-9) {
                        fail(r,TrajectoryFindingCode::GroundSpeedUnavailable,seg.segment_id,
                             "ground speed becomes unavailable within the variable-speed segment");
                        return r;
                    }
                    const double dt = ds / gs;
                    duration += dt;
                    northDisplacement += gn * dt;
                    eastDisplacement += ge * dt;
                    maximum_ground_speed = std::max(maximum_ground_speed, gs);
                }
                ground_speed = horizontal / duration;
                track = heading_deg(northDisplacement, eastDisplacement);
            } else {
                const double an=input.cruise_airspeed_mps*hn, ae=input.cruise_airspeed_mps*he;
                const double gn=an+wind->north_mps, ge=ae+wind->east_mps;
                ground_speed=std::hypot(gn,ge);
                if(ground_speed<=1e-9){
                    fail(r,TrajectoryFindingCode::GroundSpeedUnavailable,seg.segment_id,"ground speed is unavailable against wind");
                    return r;
                }
                maximum_ground_speed = ground_speed;
                duration=horizontal/ground_speed;
                track=heading_deg(gn,ge);
            }
        }
        if(std::abs(dz)>1e-9) duration=std::max(duration,std::abs(dz)/vertical_rate);
        const double climb=std::max(0.0,dz), descent=std::max(0.0,-dz);
        double cameraTriggerInterval = 0.0;
        double cameraMotionBlur = 0.0;
        if (cameraConfigured && std::find(input.camera_capture_segment_ids.begin(),
                                          input.camera_capture_segment_ids.end(), seg.segment_id)
                                   != input.camera_capture_segment_ids.end()) {
            if (horizontal <= 1e-9 || ground_speed <= 1e-9) {
                fail(r, TrajectoryFindingCode::GroundSpeedUnavailable, seg.segment_id,
                     "camera capture segment requires a positive horizontal ground speed");
                return r;
            }
            cameraTriggerInterval = input.camera_forward_trigger_spacing_m / maximum_ground_speed;
            cameraMotionBlur = maximum_ground_speed * input.camera_exposure_time_s / input.camera_gsd_m_px;
            if (cameraTriggerInterval + 1e-9 < input.camera_minimum_trigger_interval_s) {
                fail(r, TrajectoryFindingCode::CameraTriggerIntervalExceeded, seg.segment_id,
                     "wind-corrected ground speed requires a camera trigger interval shorter than the configured hardware minimum");
                return r;
            }
            if (cameraMotionBlur > input.camera_maximum_motion_blur_px + 1e-9) {
                fail(r, TrajectoryFindingCode::CameraMotionBlurExceeded, seg.segment_id,
                     "wind-corrected ground speed and selected exposure exceed the configured motion-blur tolerance");
                return r;
            }
        }
        const double segment_energy=
            horizontal*input.energy_per_horizontal_meter_wh+
            climb*input.energy_per_climb_meter_wh+
            descent*input.energy_per_descent_meter_wh+
            input.payload_power_w*duration/3600.0;

        time+=duration; energy+=segment_energy;
        if(energy+input.reserve_requirement_wh>input.usable_energy_wh){
            fail(r,TrajectoryFindingCode::InsufficientEnergyReserve,seg.segment_id,"trajectory exceeds usable energy after reserve");
            return r;
        }
        r.segments.push_back({seg.segment_id,ground_speed,track,duration,segment_energy,
                              cameraTriggerInterval,cameraMotionBlur});
        r.points.push_back({to->waypoint_id,to->position,to->altitude_m,time,energy});
    }

    // Include execution at the final waypoint as well; there is no following
    // flight segment whose forecast sample would need a shifted departure time.
    const auto& finalWaypoint = route.waypoints.back();
    if (finalWaypoint.task_execution_duration_s.has_value()) {
        time += *finalWaypoint.task_execution_duration_s;
        energy += *finalWaypoint.task_execution_energy_wh;
        r.points.back().elapsed_time_s = time;
        r.points.back().cumulative_energy_wh = energy;
        if (energy + input.reserve_requirement_wh > input.usable_energy_wh) {
            fail(r, TrajectoryFindingCode::InsufficientEnergyReserve, finalWaypoint.waypoint_id,
                 "final task execution exceeds usable energy after reserve");
            return r;
        }
    }

    r.total_time_s=time;
    r.total_energy_wh=energy;
    r.remaining_energy_wh=input.usable_energy_wh-energy;
    r.status=TrajectoryStatus::Feasible;
    return r;
}
} // namespace bluesky::planning
