#include "multi_uav_conflict.hpp"
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <limits>
#include <optional>
#include <sstream>

namespace bluesky::planning {
namespace {
constexpr double kEarthRadiusM=6371000.0;
constexpr double kPi=3.14159265358979323846;
double rad(double x){return x*kPi/180.0;}
double horizontal(const GeoPoint&a,const GeoPoint&b){
    const double lat=(rad(a.latitude_deg)+rad(b.latitude_deg))*0.5;
    const double n=rad(b.latitude_deg-a.latitude_deg)*kEarthRadiusM;
    const double e=rad(b.longitude_deg-a.longitude_deg)*kEarthRadiusM*std::cos(lat);
    return std::hypot(n,e);
}
struct Vec3 { double x,y,z; };
Vec3 positionAt(const TrajectoryPoint4D&a,const TrajectoryPoint4D&b,double t){
    const double dt=b.elapsed_time_s-a.elapsed_time_s;
    const double f=dt>0.0?(t-a.elapsed_time_s)/dt:0.0;
    const double lat=rad(a.position.latitude_deg)+(rad(b.position.latitude_deg)-rad(a.position.latitude_deg))*f;
    const double lon=rad(a.position.longitude_deg)+(rad(b.position.longitude_deg)-rad(a.position.longitude_deg))*f;
    return {lat*kEarthRadiusM, lon*kEarthRadiusM*std::cos((rad(a.position.latitude_deg)+rad(b.position.latitude_deg))*0.5),
            a.altitude_m+(b.altitude_m-a.altitude_m)*f};
}
bool intervalConflict(const TrajectoryPoint4D&a0,const TrajectoryPoint4D&a1,
                      const TrajectoryPoint4D&b0,const TrajectoryPoint4D&b1,
                      double lo,double hi,double hsep,double vsep,double& hit){
    if(hi < lo) return false;
    const Vec3 ap0=positionAt(a0,a1,lo), ap1=positionAt(a0,a1,hi);
    const Vec3 bp0=positionAt(b0,b1,lo), bp1=positionAt(b0,b1,hi);
    const double dt=hi-lo;
    const double rx0=ap0.x-bp0.x, ry0=ap0.y-bp0.y, rz0=ap0.z-bp0.z;
    const double rvx=dt>0?(ap1.x-ap0.x-bp1.x+bp0.x)/dt:0.0;
    const double rvy=dt>0?(ap1.y-ap0.y-bp1.y+bp0.y)/dt:0.0;
    const double rvz=dt>0?(ap1.z-ap0.z-bp1.z+bp0.z)/dt:0.0;

    double zlo=lo, zhi=hi;
    if(vsep<=0.0) {
        if(std::abs(rz0)>0.0) return false;
    } else if(std::abs(rvz)<1e-12) {
        if(std::abs(rz0)>=vsep) return false;
    } else {
        double t1=(-vsep-rz0)/rvz, t2=(vsep-rz0)/rvz;
        if(t1>t2) std::swap(t1,t2);
        zlo=std::max(0.0,std::min(dt,t1))+lo;
        zhi=std::max(0.0,std::min(dt,t2))+lo;
        if(zhi<zlo) return false;
    }
    const double tau0=zlo-lo, tau1=zhi-lo;
    auto h2=[&](double tau){const double x=rx0+rvx*tau,y=ry0+rvy*tau;return x*x+y*y;};
    double tau=tau0;
    const double denom=rvx*rvx+rvy*rvy;
    if(denom>1e-18) tau=std::clamp(-(rx0*rvx+ry0*rvy)/denom,tau0,tau1);
    const double hs=h2(tau);
    if(hs < hsep*hsep) { hit=lo+tau; return true; }
    return false;
}
}
MultiUavConflictResult MultiUavConflictDetector::evaluate(
    const std::vector<TrajectoryResult>& ts,
    const std::vector<MultiUavSeparationInput>& sep,
    const std::string& version){
    MultiUavConflictResult r; r.calculation_version=version;
    std::ostringstream dep; dep<<version;
    for(const auto&t:ts) dep<<"|"<<t.uav_id<<":"<<t.dependency_identity;
    for(const auto&s:sep) dep<<"|S:"<<s.uav_id<<":"<<std::setprecision(17)
        <<s.minimum_horizontal_separation_m<<","<<s.minimum_vertical_separation_m<<","<<s.temporal_tolerance_s;
    r.dependency_identity=dep.str();
    if(ts.size()<2) return r;
    for(const auto&t:ts) if(t.status!=TrajectoryStatus::Feasible || t.points.size()<2){
        r.status=MultiUavConflictStatus::Conflict;
        r.findings.push_back({MultiUavConflictFindingCode::InvalidTrajectory,t.uav_id,"",0.0,
            "all trajectories must be feasible and contain 4D points"});
    }
    if(r.status==MultiUavConflictStatus::Conflict) return r;
    auto sep_for=[&](const std::string&id){ for(const auto&s:sep) if(s.uav_id==id) return s; return MultiUavSeparationInput{}; };
    for(size_t i=0;i<ts.size();++i) for(size_t j=i+1;j<ts.size();++j){
        const auto si=sep_for(ts[i].uav_id), sj=sep_for(ts[j].uav_id);
        const double hsep=std::max(si.minimum_horizontal_separation_m,sj.minimum_horizontal_separation_m);
        const double vsep=std::max(si.minimum_vertical_separation_m,sj.minimum_vertical_separation_m);
        const double tol=std::max(si.temporal_tolerance_s,sj.temporal_tolerance_s);
        for(size_t ai=1;ai<ts[i].points.size();++ai) for(size_t bj=1;bj<ts[j].points.size();++bj){
            const auto&a0=ts[i].points[ai-1], a1=ts[i].points[ai], b0=ts[j].points[bj-1], b1=ts[j].points[bj];
            const double lo=std::max(a0.elapsed_time_s,b0.elapsed_time_s);
            const double hi=std::min(a1.elapsed_time_s,b1.elapsed_time_s);
            if(hi<lo) continue;
            double hit=0.0;
            if(intervalConflict(a0,a1,b0,b1,lo,hi,hsep,vsep,hit)){
                r.status=MultiUavConflictStatus::Conflict;
                r.findings.push_back({MultiUavConflictFindingCode::SpatialTemporalConflict,
                    ts[i].uav_id,ts[j].uav_id,hit,"continuous minimum spatial/altitude separation is violated"});
                return r;
            }
        }
    }
    return r;
}
} // namespace bluesky::planning
