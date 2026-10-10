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

std::string dependency(const Route&r,const TrajectoryPerformanceInput&i,const std::string&v){
    std::ostringstream o;
    o<<r.lineage.route_id<<'|'<<r.lineage.route_version<<'|'<<r.lineage.calculation_input_version
     <<'|'<<i.uav_id<<'|'<<i.configuration_version<<'|'<<i.performance_version
     <<'|'<<i.wind_snapshot_id<<'|'<<i.wind_snapshot_version<<'|'<<v;
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
    r.uav_id=input.uav_id;
    r.configuration_version=input.configuration_version;
    r.wind_snapshot_id=input.wind_snapshot_id;
    r.wind_snapshot_version=input.wind_snapshot_version;
    r.calculation_version=calculation_version;
    r.dependency_identity=dependency(route,input,calculation_version);

    if(input.cruise_airspeed_mps<=0.0 || input.climb_rate_mps<=0.0 ||
       input.descent_rate_mps<=0.0 || input.wind_tolerance_mps<0.0 ||
       input.energy_per_horizontal_meter_wh<0.0 ||
       input.energy_per_climb_meter_wh<0.0 ||
       input.energy_per_descent_meter_wh<0.0 ||
       input.usable_energy_wh<0.0 || input.reserve_requirement_wh<0.0){
        fail(r,TrajectoryFindingCode::InvalidPerformance,"","performance inputs must be non-negative and speeds/rates positive");
        return r;
    }

    if(route.waypoints.empty() || route.segments.size()+1!=route.waypoints.size()){
        fail(r,TrajectoryFindingCode::InvalidPerformance,"","route waypoint/segment structure is invalid");
        return r;
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
        const double dz=to->altitude_m-from->altitude_m;
        const double vertical_rate=dz>0.0?input.climb_rate_mps:(dz<0.0?input.descent_rate_mps:input.cruise_airspeed_mps);

        double duration=0.0, ground_speed=0.0, track=heading_deg(north,east);
        if(horizontal>1e-9){
            const double hn=north/horizontal, he=east/horizontal;
            const double an=input.cruise_airspeed_mps*hn, ae=input.cruise_airspeed_mps*he;
            const double gn=an+wind->north_mps, ge=ae+wind->east_mps;
            ground_speed=std::hypot(gn,ge);
            if(ground_speed<=1e-9){
                fail(r,TrajectoryFindingCode::GroundSpeedUnavailable,seg.segment_id,"ground speed is unavailable against wind");
                return r;
            }
            duration=horizontal/ground_speed;
            track=heading_deg(gn,ge);
        }
        if(std::abs(dz)>1e-9) duration=std::max(duration,std::abs(dz)/vertical_rate);
        const double climb=std::max(0.0,dz), descent=std::max(0.0,-dz);
        const double segment_energy=
            horizontal*input.energy_per_horizontal_meter_wh+
            climb*input.energy_per_climb_meter_wh+
            descent*input.energy_per_descent_meter_wh;

        time+=duration; energy+=segment_energy;
        if(energy+input.reserve_requirement_wh>input.usable_energy_wh){
            fail(r,TrajectoryFindingCode::InsufficientEnergyReserve,seg.segment_id,"trajectory exceeds usable energy after reserve");
            return r;
        }
        r.segments.push_back({seg.segment_id,ground_speed,track,duration,segment_energy});
        r.points.push_back({to->waypoint_id,to->position,to->altitude_m,time,energy});
    }

    r.total_time_s=time;
    r.total_energy_wh=energy;
    r.remaining_energy_wh=input.usable_energy_wh-energy;
    r.status=TrajectoryStatus::Feasible;
    return r;
}
} // namespace bluesky::planning
