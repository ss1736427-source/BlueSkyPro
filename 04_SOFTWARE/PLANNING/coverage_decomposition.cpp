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

namespace {
double distance(const GeoPoint& a, const GeoPoint& b) {
    const double lat=(a.latitude_deg+b.latitude_deg)*0.5*kPi/180.0;
    const double dx=(b.longitude_deg-a.longitude_deg)*kM*std::cos(lat);
    const double dy=(b.latitude_deg-a.latitude_deg)*kM;
    return std::sqrt(dx*dx+dy*dy);
}
}

CoverageTrackResult CoverageTrackGenerator::generate(const CoverageTrackInput& input) {
    CoverageTrackResult result;
    if(!input.decomposition.valid) {
        result.failure_code="INVALID_DECOMPOSITION";
        return result;
    }
    if(!std::isfinite(input.altitude_m) || input.altitude_m < 0.0) {
        result.failure_code="INVALID_TRACK_ALTITUDE";
        return result;
    }
    if(input.decomposition.cells.empty()) {
        result.failure_code="NO_PLANNING_CELL";
        return result;
    }

    result.dependency_identity =
        input.decomposition.dependency_identity + "|" +
        input.calculation_version + "|" +
        std::to_string(input.altitude_m);

    for(const auto& cell : input.decomposition.cells) {
        if(cell.polygon.size() < 3) {
            result.failure_code="INVALID_CELL_GEOMETRY";
            return result;
        }

        const double reference_lat=cell.polygon.front().latitude_deg;
        const double angle=cell.orientation_deg*kPi/180.0;
        std::vector<XY> polygon;
        polygon.reserve(cell.polygon.size());
        for(const auto& point : cell.polygon)
            polygon.push_back(rotate(project(point,reference_lat),-angle));

        double min_y=std::numeric_limits<double>::infinity();
        double max_y=-std::numeric_limits<double>::infinity();
        for(const auto& point : polygon) {
            min_y=std::min(min_y,point.y);
            max_y=std::max(max_y,point.y);
        }
        if(!(max_y>min_y)) {
            result.failure_code="DEGENERATE_CELL";
            return result;
        }

        const double scan_y=(min_y+max_y)*0.5;
        std::vector<double> intersections;
        for(std::size_t i=0;i<polygon.size();++i) {
            const XY a=polygon[i];
            const XY b=polygon[(i+1)%polygon.size()];
            if((a.y<=scan_y && b.y>scan_y) || (b.y<=scan_y && a.y>scan_y)) {
                const double t=(scan_y-a.y)/(b.y-a.y);
                intersections.push_back(a.x+t*(b.x-a.x));
            }
        }
        std::sort(intersections.begin(),intersections.end());
        if(intersections.size()<2 || intersections.size()%2!=0) {
            result.failure_code="NO_VALID_TRACK_INTERSECTION";
            return result;
        }

        std::size_t interval_index=0;
        for(std::size_t i=0;i<intersections.size();i+=2,++interval_index) {
            const XY local_start{intersections[i],scan_y};
            const XY local_end{intersections[i+1],scan_y};
            GeoPoint start=unproject(rotate(local_start,angle),reference_lat);
            GeoPoint end=unproject(rotate(local_end,angle),reference_lat);
            if(interval_index%2!=0) std::swap(start,end);

            CoverageTrack track;
            track.generation_index=result.tracks.size();
            track.track_id="MT01-TRACK-"+std::to_string(track.generation_index);
            track.cell_id=cell.cell_id;
            track.start=start;
            track.end=end;
            track.altitude_m=input.altitude_m;
            track.length_m=distance(start,end);
            if(!(track.length_m>0.0) || !std::isfinite(track.length_m)) {
                result.failure_code="INVALID_TRACK_LENGTH";
                return result;
            }
            result.tracks.push_back(std::move(track));
        }
    }

    if(result.tracks.empty()) {
        result.failure_code="NO_COVERAGE_TRACK";
        return result;
    }
    result.valid=true;
    return result;
}

} // namespace bluesky::planning
