#include "coverage_decomposition.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

namespace bluesky::planning {
namespace {
constexpr double kPi=3.14159265358979323846;
constexpr double kM=111320.0;
struct XY { double x; double y; };

XY project(const GeoPoint& p,double lat) {
    return {p.longitude_deg*kM*std::cos(lat*kPi/180.0),p.latitude_deg*kM};
}
GeoPoint unproject(XY p,double lat) {
    return {p.y/kM,p.x/(kM*std::cos(lat*kPi/180.0))};
}
XY rotate(XY p,double a) {
    const double c=std::cos(a),s=std::sin(a);
    return {c*p.x-s*p.y,s*p.x+c*p.y};
}
double area(const std::vector<XY>& p) {
    double s=0.0;
    if(p.size()<3) return 0.0;
    for(std::size_t i=0;i<p.size();++i) {
        const auto& a=p[i]; const auto& b=p[(i+1)%p.size()];
        s+=a.x*b.y-b.x*a.y;
    }
    return std::abs(s)*0.5;
}
std::vector<XY> clip(const std::vector<XY>& p,double x,bool minimum) {
    std::vector<XY> r;
    if(p.empty()) return r;
    auto in=[&](XY q){return minimum?q.x>=x:q.x<=x;};
    for(std::size_t i=0;i<p.size();++i) {
        const XY c=p[i],q=p[(i+p.size()-1)%p.size()];
        const bool ci=in(c),qi=in(q);
        if(ci!=qi) {
            const double dx=c.x-q.x;
            const double t=dx==0.0?0.0:(x-q.x)/dx;
            r.push_back({x,q.y+t*(c.y-q.y)});
        }
        if(ci) r.push_back(c);
    }
    return r;
}
std::string dependency(const CoverageDecompositionInput& i) {
    return i.geometry_revision+"|"+i.calculation_version+"|"+
        std::to_string(i.orientation.orientation_deg)+"|"+
        std::to_string(i.track_spacing_m)+"|"+
        i.environment.snapshot_id+"|"+i.environment.snapshot_version;
}
CoverageDecompositionResult fail(const CoverageDecompositionInput&i,const std::string&code) {
    CoverageDecompositionResult r; r.failure_code=code; r.dependency_identity=dependency(i); return r;
}
bool validPoint(const GeoPoint&p) {
    return std::isfinite(p.latitude_deg)&&std::isfinite(p.longitude_deg)&&
        p.latitude_deg>=-90&&p.latitude_deg<=90&&p.longitude_deg>=-180&&p.longitude_deg<=180;
}

CoverageCellConstraintState classify(const std::vector<GeoPoint>& cell,
                                     const CoverageDecompositionInput& i) {
    if(i.environment.restrictions.empty()) return CoverageCellConstraintState::Open;
    const double altitude=(i.minimum_altitude_m+i.maximum_altitude_m)*0.5;
    for(std::size_t n=0;n<cell.size();++n) {
        SpatialEdge edge{cell[n],cell[(n+1)%cell.size()],altitude,
                         i.minimum_altitude_m,i.maximum_altitude_m};
        if(!ConstrainedOpenSpace::evaluateSegment(i.environment,edge).allowed)
            return CoverageCellConstraintState::Constrained;
    }
    return CoverageCellConstraintState::Open;
}
}
CoverageDecompositionResult CoverageDecompositionEngine::decompose(const CoverageDecompositionInput&i) {
    if(i.aoi.size()<3) return fail(i,"INVALID_AOI");
    for(const auto&p:i.aoi) if(!validPoint(p)) return fail(i,"INVALID_AOI_COORDINATE");
    if(!std::isfinite(i.orientation.orientation_deg)) return fail(i,"INVALID_ORIENTATION");
    if(!std::isfinite(i.track_spacing_m)||i.track_spacing_m<=0) return fail(i,"INVALID_TRACK_SPACING");
    if(!std::isfinite(i.minimum_altitude_m)||!std::isfinite(i.maximum_altitude_m)||
       i.maximum_altitude_m<i.minimum_altitude_m) return fail(i,"INVALID_ALTITUDE_BAND");
    if(!i.environment.complete) return fail(i,"ENVIRONMENT_INCOMPLETE");

    const double lat=i.aoi.front().latitude_deg;
    const double angle=i.orientation.orientation_deg*kPi/180.0;
    std::vector<XY> rotated;
    for(const auto&p:i.aoi) rotated.push_back(rotate(project(p,lat),-angle));

    double minX=std::numeric_limits<double>::infinity();
    double maxX=-std::numeric_limits<double>::infinity();
    for(const auto&p:rotated){minX=std::min(minX,p.x);maxX=std::max(maxX,p.x);}
    if(!(maxX>minX)) return fail(i,"DEGENERATE_AOI");

    CoverageDecompositionResult r;
    r.dependency_identity=dependency(i);
    const std::size_t count=static_cast<std::size_t>(
        std::ceil((maxX-minX)/i.track_spacing_m));

    for(std::size_t n=0;n<count;++n) {
        const double x0=minX+n*i.track_spacing_m;
        const double x1=std::min(maxX,minX+(n+1)*i.track_spacing_m);
        auto cell=clip(clip(rotated,x0,true),x1,false);
        if(cell.size()<3||area(cell)<=0) continue;

        CoveragePlanningCell out;
        out.generation_index=r.cells.size();
        out.cell_id="MT01-CELL-"+std::to_string(out.generation_index);
        out.orientation_deg=i.orientation.orientation_deg;
        out.constraint_state=classify(out.polygon,i);
        out.area_m2=area(cell);
        for(const auto&p:cell) out.polygon.push_back(unproject(rotate(p,angle),lat));
        r.cells.push_back(std::move(out));
    }
    if(r.cells.empty()) return fail(i,"NO_PLANNING_CELL");
    r.valid=true;
    return r;
}
} // namespace bluesky::planning
