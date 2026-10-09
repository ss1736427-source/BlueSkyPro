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
        r.cells.push_back(std::move(out));
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
    for(const auto& track:input.tracks.tracks) {
        covered += track.length_m * input.geometry.footprint_width_m;
    }
    result.estimated_covered_area_m2=std::min(result.aoi_area_m2,covered);
    result.coverage_ratio=result.estimated_covered_area_m2/result.aoi_area_m2;

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
