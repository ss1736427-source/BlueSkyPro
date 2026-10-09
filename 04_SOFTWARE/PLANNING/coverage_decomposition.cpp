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
    return ConstrainedOpenSpace::evaluatePolygon(
        i.environment, cell, i.minimum_altitude_m, i.maximum_altitude_m).allowed
        ? CoverageCellConstraintState::Open
        : CoverageCellConstraintState::Constrained;
}

struct ScanInterval { double low; double high; };

std::vector<ScanInterval> scan_intervals(
    const std::vector<XY>& polygon, double x) {
    std::vector<double> ys;
    if(polygon.size()<3) return {};
    for(std::size_t i=0;i<polygon.size();++i) {
        const XY a=polygon[i];
        const XY b=polygon[(i+1)%polygon.size()];
        if((a.x<=x && b.x>x) || (b.x<=x && a.x>x)) {
            const double t=(x-a.x)/(b.x-a.x);
            ys.push_back(a.y+t*(b.y-a.y));
        }
    }
    std::sort(ys.begin(),ys.end());
    ys.erase(std::unique(ys.begin(),ys.end(),
        [](double a,double b){ return std::abs(a-b)<1e-8; }),ys.end());
    std::vector<ScanInterval> result;
    for(std::size_t i=0;i+1<ys.size();i+=2) {
        if(ys[i+1]>ys[i]) result.push_back({ys[i],ys[i+1]});
    }
    return result;
}

std::vector<ScanInterval> subtract_intervals(
    const std::vector<ScanInterval>& subject,
    const std::vector<ScanInterval>& restriction) {
    std::vector<ScanInterval> result;
    for(const auto& source:subject) {
        std::vector<ScanInterval> remaining{source};
        for(const auto& cut:restriction) {
            std::vector<ScanInterval> next;
            for(const auto& part:remaining) {
                if(cut.high<=part.low || cut.low>=part.high) {
                    next.push_back(part);
                    continue;
                }
                if(cut.low>part.low)
                    next.push_back({part.low,std::min(cut.low,part.high)});
                if(cut.high<part.high)
                    next.push_back({std::max(cut.high,part.low),part.high});
            }
            remaining=std::move(next);
            if(remaining.empty()) break;
        }
        for(const auto& part:remaining)
            if(part.high>part.low) result.push_back(part);
    }
    return result;
}

bool segment_intersection_x(const XY& a,const XY& b,
                            const XY& c,const XY& d,double& x) {
    const double r_x=b.x-a.x, r_y=b.y-a.y;
    const double s_x=d.x-c.x, s_y=d.y-c.y;
    const double denom=r_x*s_y-r_y*s_x;
    const double q_x=c.x-a.x, q_y=c.y-a.y;
    if(std::abs(denom)<1e-10) return false;
    const double t=(q_x*s_y-q_y*s_x)/denom;
    const double u=(q_x*r_y-q_y*r_x)/denom;
    if(t<0.0 || t>1.0 || u<0.0 || u>1.0) return false;
    x=a.x+t*r_x;
    return std::isfinite(x);
}

bool valid_polygon_xy(const std::vector<XY>& polygon) {
    return polygon.size()>=3 && area(polygon)>1e-6;
}
}

CoveragePolygonSplitResult CoveragePolygonSplitter::split(
    const CoveragePolygonSplitInput& input) {
    CoveragePolygonSplitResult result;
    result.dependency_identity =
        input.restriction_id+"|"+input.source_id+"|"+input.calculation_version;

    if(input.subject_polygon.size()<3) {
        result.failure_code="INVALID_SUBJECT_POLYGON";
        return result;
    }
    if(input.restriction_polygon.size()<3) {
        result.failure_code="INVALID_RESTRICTION_POLYGON";
        return result;
    }

    const double reference_lat=input.subject_polygon.front().latitude_deg;
    std::vector<XY> subject, restriction;
    for(const auto& p:input.subject_polygon) subject.push_back(project(p,reference_lat));
    for(const auto& p:input.restriction_polygon) restriction.push_back(project(p,reference_lat));
    if(!valid_polygon_xy(subject) || !valid_polygon_xy(restriction)) {
        result.failure_code="DEGENERATE_POLYGON";
        return result;
    }

    std::vector<double> xs;
    for(const auto& p:subject) xs.push_back(p.x);
    for(const auto& p:restriction) xs.push_back(p.x);
    for(std::size_t i=0;i<subject.size();++i) {
        const auto a=subject[i], b=subject[(i+1)%subject.size()];
        for(std::size_t j=0;j<restriction.size();++j) {
            const auto c=restriction[j], d=restriction[(j+1)%restriction.size()];
            double x=0.0;
            if(segment_intersection_x(a,b,c,d,x)) xs.push_back(x);
        }
    }
    std::sort(xs.begin(),xs.end());
    xs.erase(std::unique(xs.begin(),xs.end(),
        [](double a,double b){ return std::abs(a-b)<1e-7; }),xs.end());

    const auto to_geo=[&](const std::vector<XY>& polygon) {
        std::vector<GeoPoint> out;
        for(const auto& p:polygon) out.push_back(unproject(p,reference_lat));
        return out;
    };

    bool split_occurred=false;
    for(std::size_t i=0;i+1<xs.size();++i) {
        const double x0=xs[i], x1=xs[i+1];
        if(!(x1>x0)) continue;
        const double xm=(x0+x1)*0.5;
        const double delta=(x1-x0)*1e-6;
        const double xl=x0+delta;
        const double xr=x1-delta;

        const auto subject_mid=scan_intervals(subject,xm);
        const auto restriction_mid=scan_intervals(restriction,xm);
        const auto remaining_mid=subtract_intervals(subject_mid,restriction_mid);
        if(remaining_mid.empty()) continue;

        const auto left=subtract_intervals(
            scan_intervals(subject,xl),scan_intervals(restriction,xl));
        const auto right=subtract_intervals(
            scan_intervals(subject,xr),scan_intervals(restriction,xr));
        if(left.size()!=remaining_mid.size() || right.size()!=remaining_mid.size()) {
            result.failure_code="SPLIT_TOPOLOGY_AMBIGUOUS";
            return result;
        }

        auto extrapolate=[&](const ScanInterval& sample,
                             const ScanInterval& middle,
                             double sample_x,double target_x) {
            const double denominator=xm-sample_x;
            const double lower=sample.low+
                (middle.low-sample.low)*(target_x-sample_x)/denominator;
            const double upper=sample.high+
                (middle.high-sample.high)*(target_x-sample_x)/denominator;
            return ScanInterval{lower,upper};
        };

        for(std::size_t n=0;n<remaining_mid.size();++n) {
            const auto left_at_boundary=extrapolate(left[n],remaining_mid[n],xl,x0);
            const auto right_at_boundary=extrapolate(right[n],remaining_mid[n],xr,x1);
            std::vector<XY> piece{
                {x0,left_at_boundary.low},{x1,right_at_boundary.low},
                {x1,right_at_boundary.high},{x0,left_at_boundary.high}};
            if(!valid_polygon_xy(piece)) continue;
            CoveragePolygonSplitPiece out;
            out.polygon=to_geo(piece);
            out.restriction_id=input.restriction_id;
            out.source_id=input.source_id;
            result.pieces.push_back(std::move(out));
        }

        if(remaining_mid.size()!=subject_mid.size())
            split_occurred=true;
    }

    if(result.pieces.empty()) {
        result.failure_code="NO_SPLIT_PIECES";
        return result;
    }

    if(!split_occurred) {
        result.pieces.clear();
        result.pieces.push_back({to_geo(subject),"",""});
    }
    result.valid=true;
    return result;
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
        out.area_m2=area(cell);
        for(const auto&p:cell) out.polygon.push_back(unproject(rotate(p,angle),lat));
        out.constraint_state=classify(out.polygon,i);
        std::vector<CoveragePlanningCell> fragments;
        fragments.push_back(std::move(out));
        for(const auto& restriction:i.environment.restrictions) {
            if(!restriction.active ||
               restriction.geometry_type!=RestrictionGeometryType::Polygon ||
               restriction.polygon.size()<3) continue;
            std::vector<CoveragePlanningCell> next;
            for(const auto& fragment:fragments) {
                const auto check=ConstrainedOpenSpace::evaluatePolygon(
                    i.environment,fragment.polygon,
                    i.minimum_altitude_m,i.maximum_altitude_m);
                if(check.allowed) { next.push_back(fragment); continue; }
                CoveragePolygonSplitInput si;
                si.subject_polygon=fragment.polygon;
                si.restriction_polygon=restriction.polygon;
                si.restriction_id=restriction.restriction_id;
                si.source_id=restriction.source_id;
                si.calculation_version=i.calculation_version;
                const auto sr=CoveragePolygonSplitter::split(si);
                if(!sr.valid) return fail(i,"POLYGON_SPLIT_FAILED");
                for(const auto& piece:sr.pieces) {
                    if(piece.polygon.size()<3) continue;
                    auto cell=fragment;
                    cell.polygon=piece.polygon;
                    const double ref=piece.polygon.front().latitude_deg;
                    std::vector<XY> local;
                    for(const auto& p:piece.polygon) local.push_back(project(p,ref));
                    cell.area_m2=area(local);
                    cell.constraint_state=CoverageCellConstraintState::Open;
                    next.push_back(std::move(cell));
                }
            }
            fragments=std::move(next);
            if(fragments.empty()) break;
        }
        for(auto& fragment:fragments) {
            fragment.generation_index=r.cells.size();
            fragment.cell_id="MT01-CELL-"+std::to_string(fragment.generation_index);
            r.cells.push_back(std::move(fragment));
        }
    }
    if(r.cells.empty()) return fail(i,"NO_PLANNING_CELL");
    r.valid=true;
    return r;
}

std::vector<std::pair<double,double>> free_scan_intervals(
    const std::vector<double>& boundaries,
    const XY& base_start,
    const XY& base_end,
    double altitude_m,
    const ConstrainedEnvironmentSnapshot& environment,
    double reference_lat) {
    std::vector<std::pair<double,double>> free;
    if(boundaries.size()<2) return free;
    for(std::size_t i=0;i+1<boundaries.size();++i) {
        const double left=boundaries[i], right=boundaries[i+1];
        if(!(right>left)) continue;
        const double t0=(left-base_start.x)/(base_end.x-base_start.x);
        const double t1=(right-base_start.x)/(base_end.x-base_start.x);
        const GeoPoint a=unproject(
            {base_start.x+(base_end.x-base_start.x)*t0,
             base_start.y+(base_end.y-base_start.y)*t0},reference_lat);
        const GeoPoint b=unproject(
            {base_start.x+(base_end.x-base_start.x)*t1,
             base_start.y+(base_end.y-base_start.y)*t1},reference_lat);
        const auto check=ConstrainedOpenSpace::evaluateSegment(
            environment,{a,b,altitude_m,altitude_m,altitude_m});
        if(check.allowed) free.push_back({left,right});
    }
    return free;
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
            std::vector<double> boundaries{intersections[i],intersections[i+1]};
            const XY base_start{intersections[i],scan_y};
            const XY base_end{intersections[i+1],scan_y};
            for(const auto& restriction:input.decomposition.environment.restrictions) {
                if(!restriction.active) continue;
                if(restriction.geometry_type==RestrictionGeometryType::Polygon &&
                   restriction.polygon.size()>=3) {
                    for(std::size_t e=0;e<restriction.polygon.size();++e) {
                        const XY a=rotate(project(restriction.polygon[e],reference_lat),-angle);
                        const XY b=rotate(project(
                            restriction.polygon[(e+1)%restriction.polygon.size()],reference_lat),-angle);
                        if((a.y<=scan_y && b.y>scan_y) || (b.y<=scan_y && a.y>scan_y)) {
                            const double t=(scan_y-a.y)/(b.y-a.y);
                            const double x=a.x+t*(b.x-a.x);
                            if(x>intersections[i] && x<intersections[i+1]) boundaries.push_back(x);
                        }
                    }
                } else if(restriction.geometry_type==RestrictionGeometryType::Circle &&
                          restriction.radius_m>0.0) {
                    const XY center=rotate(project(restriction.center,reference_lat),-angle);
                    const double dy=scan_y-center.y;
                    const double d2=restriction.radius_m*restriction.radius_m-dy*dy;
                    if(d2>=0.0) {
                        const double dx=std::sqrt(d2);
                        if(center.x-dx>intersections[i] && center.x-dx<intersections[i+1]) boundaries.push_back(center.x-dx);
                        if(center.x+dx>intersections[i] && center.x+dx<intersections[i+1]) boundaries.push_back(center.x+dx);
                    }
                }
            }
            std::sort(boundaries.begin(),boundaries.end());
            boundaries.erase(std::unique(boundaries.begin(),boundaries.end(),
                [](double a,double b){return std::abs(a-b)<1e-7;}),boundaries.end());
            const auto free=free_scan_intervals(
                boundaries,base_start,base_end,input.altitude_m,
                input.decomposition.environment,reference_lat);
            for(const auto& segment:free) {
                const XY local_start{segment.first,scan_y};
                const XY local_end{segment.second,scan_y};
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
    }

    if(result.tracks.empty()) {
        result.failure_code="NO_COVERAGE_TRACK";
        return result;
    }
    result.valid=true;
    return result;
}


namespace {
double point_segment_distance(const GeoPoint& p,const GeoPoint& a,const GeoPoint& b) {
    const double lat=(a.latitude_deg+b.latitude_deg+p.latitude_deg)*kPi/540.0;
    const double scale_x=kM*std::cos(lat);
    const double px=p.longitude_deg*scale_x, py=p.latitude_deg*kM;
    const double ax=a.longitude_deg*scale_x, ay=a.latitude_deg*kM;
    const double bx=b.longitude_deg*scale_x, by=b.latitude_deg*kM;
    const double dx=bx-ax, dy=by-ay;
    const double denom=dx*dx+dy*dy;
    double t=denom>0.0?((px-ax)*dx+(py-ay)*dy)/denom:0.0;
    t=std::max(0.0,std::min(1.0,t));
    const double qx=ax+t*dx, qy=ay+t*dy;
    const double ex=px-qx, ey=py-qy;
    return std::sqrt(ex*ex+ey*ey);
}
double boundary_distance(const GeoPoint& p,const CoveragePlanningCell& cell) {
    double best=std::numeric_limits<double>::infinity();
    for(std::size_t i=0;i<cell.polygon.size();++i)
        best=std::min(best,point_segment_distance(p,cell.polygon[i],
                                                   cell.polygon[(i+1)%cell.polygon.size()]));
    return best;
}
}

CoverageEdgeResult CoverageEdgeEngine::evaluate(
    const CoverageTrackInput& input,const CoverageTrackResult& tracks) {
    CoverageEdgeResult result;
    if(!input.decomposition.valid || !tracks.valid) {
        result.failure_code="INVALID_TRACK_INPUT";
        return result;
    }
    if(!std::isfinite(input.footprint_width_m) || !std::isfinite(input.footprint_height_m) ||
       input.footprint_width_m<=0.0 || input.footprint_height_m<=0.0) {
        result.failure_code="INVALID_FOOTPRINT";
        return result;
    }

    result.dependency_identity=tracks.dependency_identity+"|EDGE|"+
        std::to_string(input.footprint_width_m)+"|"+
        std::to_string(input.footprint_height_m);

    for(const auto& track:tracks.tracks) {
        const auto cell_it=std::find_if(
            input.decomposition.cells.begin(),input.decomposition.cells.end(),
            [&](const CoveragePlanningCell& c){return c.cell_id==track.cell_id;});
        if(cell_it==input.decomposition.cells.end()) {
            result.failure_code="TRACK_CELL_NOT_FOUND";
            return result;
        }

        CoverageEdgeGap gap;
        gap.cell_id=track.cell_id;
        gap.track_id=track.track_id;
        gap.start_margin_m=boundary_distance(track.start,*cell_it);
        gap.end_margin_m=boundary_distance(track.end,*cell_it);

        const double required_margin=input.footprint_height_m*0.5;
        gap.start_gap=gap.start_margin_m>required_margin;
        gap.end_gap=gap.end_margin_m>required_margin;
        if(gap.start_gap || gap.end_gap) result.gaps.push_back(gap);

        result.evaluated_tracks.push_back(track);
    }

    result.valid=true;
    return result;
}


AcquisitionEventResult AcquisitionEventValidator::generate(
    const AcquisitionEventInput& input) {
    AcquisitionEventResult result;
    if(!input.tracks.valid) {
        result.failure_code="INVALID_TRACK_RESULT";
        return result;
    }
    if(!input.geometry.valid) {
        result.failure_code="INVALID_ACQUISITION_GEOMETRY";
        return result;
    }
    if(input.tracks.tracks.empty()) {
        result.failure_code="NO_TRACKS";
        return result;
    }

    result.dependency_identity =
        input.tracks.dependency_identity + "|" +
        input.geometry.dependency_identity + "|" +
        input.calculation_version;

    for(const auto& track : input.tracks.tracks) {
        if(!(track.length_m > 0.0) || !std::isfinite(track.length_m)) {
            result.failure_code="INVALID_TRACK_LENGTH";
            return result;
        }

        AcquisitionEvent event;
        event.generation_index=result.events.size();
        event.event_id="MT01-EVENT-"+std::to_string(event.generation_index);
        event.track_id=track.track_id;
        event.position=track.start;
        event.camera_ground_distance_m =
            input.geometry.footprint_height_m > 0.0
                ? input.geometry.camera_ground_distance_m
                : 0.0;
        event.gsd_width_m_per_px=input.geometry.gsd_width_m_per_px;
        event.gsd_height_m_per_px=input.geometry.gsd_height_m_per_px;
        event.footprint_width_m=input.geometry.footprint_width_m;
        event.footprint_height_m=input.geometry.footprint_height_m;
        event.trigger_interval_s=input.geometry.trigger_interval_s;
        event.sensor_state_valid=true;
        event.trigger_state_valid=true;
        result.events.push_back(std::move(event));
    }

    result.valid=true;
    return result;
}


namespace {
double exact_footprint_union_intersection_area(
    const std::vector<GeoPoint>& aoi,
    const std::vector<CoverageTrack>& tracks,
    double footprint_width_m,
    double footprint_height_m,
    double orientation_deg) {
    if(aoi.size()<3 || tracks.empty() ||
       !(footprint_width_m>0.0) || !(footprint_height_m>0.0))
        return 0.0;

    const double reference_lat=aoi.front().latitude_deg;
    const double angle=orientation_deg*kPi/180.0;
    const double c=std::cos(-angle), s=std::sin(-angle);
    const auto rotate_local=[&](const GeoPoint& p) {
        const XY q=project(p,reference_lat);
        return XY{c*q.x-s*q.y,s*q.x+c*q.y};
    };

    struct Rect { double x0,x1,y0,y1; };
    std::vector<Rect> rects;
    std::vector<double> xs;
    rects.reserve(tracks.size());
    xs.reserve(tracks.size()*2);

    for(const auto& track:tracks) {
        const XY a=rotate_local(track.start);
        const XY z=rotate_local(track.end);
        const double minx=std::min(a.x,z.x)-footprint_height_m*0.5;
        const double maxx=std::max(a.x,z.x)+footprint_height_m*0.5;
        const double miny=std::min(a.y,z.y)-footprint_width_m*0.5;
        const double maxy=std::max(a.y,z.y)+footprint_width_m*0.5;
        if(!(maxx>minx) || !(maxy>miny)) continue;
        rects.push_back({minx,maxx,miny,maxy});
        xs.push_back(minx);
        xs.push_back(maxx);
    }
    if(rects.empty()) return 0.0;

    std::sort(xs.begin(),xs.end());
    xs.erase(std::unique(xs.begin(),xs.end(),
        [](double a,double b){ return std::abs(a-b)<1e-9; }),xs.end());

    std::vector<XY> subject;
    subject.reserve(aoi.size());
    for(const auto& p:aoi) subject.push_back(rotate_local(p));

    auto clip_halfplane=[&](const std::vector<XY>& polygon,
                            double boundary,
                            int axis,
                            bool minimum) {
        std::vector<XY> out;
        if(polygon.empty()) return out;
        auto inside=[&](const XY& q) {
            const double value=axis==0?q.x:q.y;
            return minimum ? value>=boundary : value<=boundary;
        };
        for(std::size_t i=0;i<polygon.size();++i) {
            const XY current=polygon[i];
            const XY previous=polygon[(i+polygon.size()-1)%polygon.size()];
            const bool current_inside=inside(current);
            const bool previous_inside=inside(previous);
            if(current_inside!=previous_inside) {
                const double pv=axis==0?previous.x:previous.y;
                const double cv=axis==0?current.x:current.y;
                const double denominator=cv-pv;
                const double t=denominator==0.0 ? 0.0 : (boundary-pv)/denominator;
                out.push_back({
                    previous.x+t*(current.x-previous.x),
                    previous.y+t*(current.y-previous.y)});
            }
            if(current_inside) out.push_back(current);
        }
        return out;
    };

    auto clipped_area=[&](double x0,double x1,double y0,double y1) {
        auto polygon=clip_halfplane(subject,x0,0,true);
        polygon=clip_halfplane(polygon,x1,0,false);
        polygon=clip_halfplane(polygon,y0,1,true);
        polygon=clip_halfplane(polygon,y1,1,false);
        return area(polygon);
    };

    double total=0.0;
    for(std::size_t ix=0;ix+1<xs.size();++ix) {
        const double xa=xs[ix], xb=xs[ix+1];
        if(!(xb>xa)) continue;

        std::vector<std::pair<double,double>> intervals;
        for(const auto& rect:rects) {
            if(rect.x0<xb && rect.x1>xa)
                intervals.push_back({rect.y0,rect.y1});
        }
        if(intervals.empty()) continue;

        std::sort(intervals.begin(),intervals.end());
        std::vector<std::pair<double,double>> merged;
        for(const auto& interval:intervals) {
            if(merged.empty() || interval.first>merged.back().second)
                merged.push_back(interval);
            else
                merged.back().second=std::max(merged.back().second,interval.second);
        }
        for(const auto& interval:merged)
            total+=clipped_area(xa,xb,interval.first,interval.second);
    }
    return total;
}


double polygon_area_geo(const std::vector<GeoPoint>& polygon);

bool point_in_polygon(const GeoPoint& p,const std::vector<GeoPoint>& polygon) {
    bool inside=false;
    if(polygon.size()<3) return false;
    for(std::size_t i=0,j=polygon.size()-1;i<polygon.size();j=i++) {
        const auto& a=polygon[i];
        const auto& b=polygon[j];
        if((a.latitude_deg>p.latitude_deg)!=(b.latitude_deg>p.latitude_deg)) {
            const double x=a.longitude_deg+
                (b.longitude_deg-a.longitude_deg)*
                (p.latitude_deg-a.latitude_deg)/(b.latitude_deg-a.latitude_deg);
            if(p.longitude_deg<x) inside=!inside;
        }
    }
    return inside;
}

double point_segment_distance_m(const GeoPoint& p,const GeoPoint& a,const GeoPoint& b) {
    const double lat=(a.latitude_deg+b.latitude_deg+p.latitude_deg)*kPi/540.0;
    const double sx=kM*std::cos(lat);
    const double px=p.longitude_deg*sx, py=p.latitude_deg*kM;
    const double ax=a.longitude_deg*sx, ay=a.latitude_deg*kM;
    const double bx=b.longitude_deg*sx, by=b.latitude_deg*kM;
    const double dx=bx-ax, dy=by-ay;
    const double d2=dx*dx+dy*dy;
    double t=d2>0.0?((px-ax)*dx+(py-ay)*dy)/d2:0.0;
    t=std::max(0.0,std::min(1.0,t));
    const double ex=px-(ax+t*dx), ey=py-(ay+t*dy);
    return std::sqrt(ex*ex+ey*ey);
}

bool touches_boundary(const std::vector<GeoPoint>& component,
                      const std::vector<GeoPoint>& aoi) {
    constexpr double kBoundaryToleranceM=0.05;
    for(const auto& p:component)
        for(std::size_t i=0;i<aoi.size();++i)
            if(point_segment_distance_m(
                   p,aoi[i],aoi[(i+1)%aoi.size()])<=kBoundaryToleranceM)
                return true;
    return false;
}

bool overlaps_polygon_restriction(const std::vector<GeoPoint>& component,
                                   const std::vector<GeoPoint>& restriction) {
    if(component.size()<3||restriction.size()<3) return false;
    for(const auto& p:component)
        if(point_in_polygon(p,restriction)) return true;
    for(const auto& p:restriction)
        if(point_in_polygon(p,component)) return true;
    return false;
}

bool overlaps_circle_restriction(const std::vector<GeoPoint>& component,
                                  const SpatialRestriction& restriction) {
    if(component.size()<3||restriction.radius_m<=0.0) return false;
    for(const auto& p:component) {
        const double lat=(p.latitude_deg+restriction.center.latitude_deg)*0.5*kPi/180.0;
        const double dx=(p.longitude_deg-restriction.center.longitude_deg)*
            kM*std::cos(lat);
        const double dy=(p.latitude_deg-restriction.center.latitude_deg)*kM;
        if(std::sqrt(dx*dx+dy*dy)<=restriction.radius_m) return true;
    }
    if(point_in_polygon(restriction.center,component)) return true;
    for(std::size_t i=0;i<component.size();++i)
        if(point_segment_distance_m(
               restriction.center,component[i],
               component[(i+1)%component.size()])<=restriction.radius_m)
            return true;
    return false;
}

UncoveredGeometryClassification classify_uncovered_component(
    const std::vector<GeoPoint>& component,
    const std::vector<GeoPoint>& aoi,
    const ConstrainedEnvironmentSnapshot& environment,
    std::vector<std::string>& source_ids) {
    for(const auto& restriction:environment.restrictions) {
        if(!restriction.active) continue;
        bool overlap=false;
        if(restriction.geometry_type==RestrictionGeometryType::Polygon)
            overlap=overlaps_polygon_restriction(component,restriction.polygon);
        else if(restriction.geometry_type==RestrictionGeometryType::Circle)
            overlap=overlaps_circle_restriction(component,restriction);
        if(overlap) {
            source_ids.push_back(restriction.restriction_id);
            return UncoveredGeometryClassification::ExclusionInduced;
        }
    }
    if(touches_boundary(component,aoi))
        return UncoveredGeometryClassification::BoundaryGap;
    return UncoveredGeometryClassification::UnclassifiedSourceNotBound;
}

std::vector<std::vector<XY>> subtract_rect_from_polygons(
    const std::vector<std::vector<XY>>& input,
    double x0,double x1,double y0,double y1) {
    std::vector<std::vector<XY>> output;
    auto clip=[&](const std::vector<XY>& polygon,double boundary,int axis,bool minimum) {
        std::vector<XY> out;
        if(polygon.empty()) return out;
        auto inside=[&](const XY& q) {
            const double value=axis==0?q.x:q.y;
            return minimum ? value>=boundary : value<=boundary;
        };
        for(std::size_t i=0;i<polygon.size();++i) {
            const XY current=polygon[i];
            const XY previous=polygon[(i+polygon.size()-1)%polygon.size()];
            const bool ci=inside(current), pi=inside(previous);
            if(ci!=pi) {
                const double pv=axis==0?previous.x:previous.y;
                const double cv=axis==0?current.x:current.y;
                const double denominator=cv-pv;
                const double t=denominator==0.0 ? 0.0 : (boundary-pv)/denominator;
                out.push_back({previous.x+t*(current.x-previous.x),
                               previous.y+t*(current.y-previous.y)});
            }
            if(ci) out.push_back(current);
        }
        return out;
    };
    for(const auto& polygon:input) {
        auto add=[&](std::vector<XY> p) {
            if(p.size()>=3 && area(p)>0.0) output.push_back(std::move(p));
        };
        add(clip(polygon,x0,0,false));
        add(clip(polygon,x1,0,true));
        auto middle=clip(polygon,x0,0,true);
        middle=clip(middle,x1,0,false);
        add(clip(middle,y0,1,false));
        add(clip(middle,y1,1,true));
    }
    return output;
}

std::vector<std::vector<GeoPoint>> exact_uncovered_geometry(
    const std::vector<GeoPoint>& aoi,
    const std::vector<CoverageTrack>& tracks,
    double footprint_width_m,
    double footprint_height_m,
    double orientation_deg) {
    if(aoi.size()<3) return {};
    const double reference_lat=aoi.front().latitude_deg;
    const double angle=orientation_deg*kPi/180.0;
    const double c=std::cos(-angle), s=std::sin(-angle);
    const auto rotate_local=[&](const GeoPoint& p) {
        const XY q=project(p,reference_lat);
        return XY{c*q.x-s*q.y,s*q.x+c*q.y};
    };
    const auto unrotate_geo=[&](const XY& p) {
        const double cc=std::cos(angle), ss=std::sin(angle);
        const XY q{cc*p.x-ss*p.y,ss*p.x+cc*p.y};
        return unproject(q,reference_lat);
    };

    std::vector<std::vector<XY>> uncovered;
    std::vector<XY> aoi_local;
    aoi_local.reserve(aoi.size());
    for(const auto& point:aoi) aoi_local.push_back(rotate_local(point));
    uncovered.push_back(std::move(aoi_local));

    if(!(footprint_width_m>0.0) || !(footprint_height_m>0.0) || tracks.empty())
        return {{aoi}};

    for(const auto& track:tracks) {
        const XY a=rotate_local(track.start);
        const XY z=rotate_local(track.end);
        const double x0=std::min(a.x,z.x)-footprint_height_m*0.5;
        const double x1=std::max(a.x,z.x)+footprint_height_m*0.5;
        const double y0=std::min(a.y,z.y)-footprint_width_m*0.5;
        const double y1=std::max(a.y,z.y)+footprint_width_m*0.5;
        if(x1>x0 && y1>y0)
            uncovered=subtract_rect_from_polygons(uncovered,x0,x1,y0,y1);
        if(uncovered.empty()) break;
    }

    std::vector<std::vector<GeoPoint>> result;
    for(const auto& polygon:uncovered) {
        std::vector<GeoPoint> geo;
        geo.reserve(polygon.size());
        for(const auto& point:polygon) geo.push_back(unrotate_geo(point));
        if(geo.size()>=3 && polygon_area_geo(geo)>0.0) result.push_back(std::move(geo));
    }
    return result;
}

double polygon_area_geo(const std::vector<GeoPoint>& polygon) {
    if(polygon.size()<3) return 0.0;
    const double lat=polygon.front().latitude_deg*kPi/180.0;
    std::vector<XY> p;
    p.reserve(polygon.size());
    for(const auto& point:polygon) p.push_back(project(point,polygon.front().latitude_deg));
    return area(p);
}
}
MappingQualityResult MappingQualityEngine::evaluate(const MappingQualityInput& input) {
    MappingQualityResult result;
    if(input.aoi.size()<3) { result.failure_code="INVALID_AOI"; return result; }
    if(!input.decomposition.valid) { result.failure_code="INVALID_DECOMPOSITION"; return result; }
    if(!input.tracks.valid) { result.failure_code="INVALID_TRACKS"; return result; }
    if(!input.events.valid) { result.failure_code="INVALID_EVENTS"; return result; }
    if(!input.geometry.valid) { result.failure_code="INVALID_GEOMETRY"; return result; }
    if(input.events.events.empty()) { result.failure_code="NO_ACQUISITION_EVENTS"; return result; }

    result.dependency_identity=input.decomposition.dependency_identity+"|"+
        input.tracks.dependency_identity+"|"+input.events.dependency_identity+"|"+
        input.geometry.dependency_identity+"|"+input.calculation_version;
    result.aoi_area_m2=polygon_area_geo(input.aoi);
    if(!(result.aoi_area_m2>0.0) || !std::isfinite(result.aoi_area_m2)) {
        result.failure_code="INVALID_AOI_AREA"; return result;
    }

    double covered=0.0;
    for(const auto& track:input.tracks.tracks)
        covered += track.length_m * input.geometry.footprint_width_m;
    result.estimated_covered_area_m2=std::min(result.aoi_area_m2,covered);
    const double union_area=exact_footprint_union_intersection_area(
        input.aoi,
        input.tracks.tracks,
        input.geometry.footprint_width_m,
        input.geometry.footprint_height_m,
        input.decomposition.cells.empty()
            ? 0.0 : input.decomposition.cells.front().orientation_deg);
    result.footprint_union_area_m2=union_area;
    result.coverage_ratio=std::min(result.aoi_area_m2,union_area)/result.aoi_area_m2;
    result.uncovered_geometry=exact_uncovered_geometry(
        input.aoi,
        input.tracks.tracks,
        input.geometry.footprint_width_m,
        input.geometry.footprint_height_m,
        input.decomposition.cells.empty()
            ? 0.0 : input.decomposition.cells.front().orientation_deg);
    for(const auto& polygon:result.uncovered_geometry) {
        const double polygon_area=polygon_area_geo(polygon);
        result.uncovered_area_m2+=polygon_area;
        UncoveredGeometryComponent component;
        component.polygon=polygon;
        component.area_m2=polygon_area;
        component.classification=classify_uncovered_component(
            polygon,input.aoi,input.environment,component.source_ids);
        result.uncovered_components.push_back(std::move(component));
    }

    result.min_gsd_m_per_px=std::numeric_limits<double>::infinity();
    result.max_gsd_m_per_px=0.0;
    result.min_frontal_overlap_ratio=std::numeric_limits<double>::infinity();
    result.min_side_overlap_ratio=std::numeric_limits<double>::infinity();

    for(const auto& event:input.events.events) {
        if(!event.sensor_state_valid || !event.trigger_state_valid) ++result.invalid_event_count;
        result.min_gsd_m_per_px=std::min(result.min_gsd_m_per_px,
                                         std::min(event.gsd_width_m_per_px,event.gsd_height_m_per_px));
        result.max_gsd_m_per_px=std::max(result.max_gsd_m_per_px,
                                         std::max(event.gsd_width_m_per_px,event.gsd_height_m_per_px));
    }
    result.min_frontal_overlap_ratio=input.geometry.frontal_overlap_ratio;
    result.min_side_overlap_ratio=input.geometry.side_overlap_ratio;
    result.gate_passed=result.invalid_event_count==0 &&
        result.coverage_ratio>0.0 &&
        std::isfinite(result.min_gsd_m_per_px) &&
        std::isfinite(result.max_gsd_m_per_px);
    result.valid=true;
    return result;
}

} // namespace bluesky::planning
