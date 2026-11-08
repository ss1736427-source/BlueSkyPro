#include "coverage_path_planner.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <utility>

namespace bluesky::planning {
namespace {
constexpr double kPi = 3.14159265358979323846;
constexpr double kEarthRadiusM = 6371008.8;
struct XY { double x; double y; };
struct Interval { double lo; double hi; };

bool finite(double v) { return std::isfinite(v); }

bool validRing(const std::vector<GeoPoint>& ring) {
    if (ring.size() < 3) return false;
    for (const auto& p : ring)
        if (!finite(p.latitude_deg) || !finite(p.longitude_deg) ||
            p.latitude_deg < -90.0 || p.latitude_deg > 90.0 ||
            p.longitude_deg < -180.0 || p.longitude_deg > 180.0) return false;
    return true;
}

XY project(const GeoPoint& p, double lat0, double lon0) {
    const double c = std::cos(lat0 * kPi / 180.0);
    return {kEarthRadiusM * (p.longitude_deg-lon0) * kPi/180.0 * c,
            kEarthRadiusM * (p.latitude_deg-lat0) * kPi/180.0};
}
GeoPoint unproject(const XY& p, double lat0, double lon0) {
    const double c = std::cos(lat0 * kPi / 180.0);
    return {lat0 + p.y / kEarthRadiusM * 180.0/kPi,
            lon0 + p.x / (kEarthRadiusM*c) * 180.0/kPi};
}
std::vector<double> intersections(const std::vector<XY>& ring, double y) {
    std::vector<double> xs;
    for (std::size_t i=0,j=ring.size()-1;i<ring.size();j=i++) {
        const auto& a=ring[j]; const auto& b=ring[i];
        if ((a.y <= y && b.y > y) || (b.y <= y && a.y > y))
            xs.push_back(a.x + (y-a.y)*(b.x-a.x)/(b.y-a.y));
    }
    std::sort(xs.begin(),xs.end());
    return xs;
}
std::vector<Interval> intervalsFor(const std::vector<XY>& ring, double y) {
    const auto xs=intersections(ring,y);
    std::vector<Interval> out;
    for (std::size_t i=1;i<xs.size();i+=2)
        if (xs[i]-xs[i-1] > 0.01) out.push_back({xs[i-1],xs[i]});
    return out;
}
std::vector<Interval> subtractIntervals(std::vector<Interval> base,
                                        const std::vector<Interval>& cuts) {
    for (const auto& cut:cuts) {
        std::vector<Interval> next;
        for (const auto& b:base) {
            if (cut.hi<=b.lo || cut.lo>=b.hi) { next.push_back(b); continue; }
            if (cut.lo>b.lo) next.push_back({b.lo,std::min(cut.lo,b.hi)});
            if (cut.hi<b.hi) next.push_back({std::max(cut.hi,b.lo),b.hi});
        }
        base=std::move(next);
    }
    return base;
}
}

CoveragePlanResult CoveragePathPlanner::generate(const CoverageMissionInput& in) {
    CoveragePlanResult out;
    auto fail=[&](const std::string& why){out.valid=false;out.status="INVALID_INPUT";out.diagnostics.push_back(why);return out;};
    if (!validRing(in.area_of_interest)) return fail("AOI must be a valid polygon ring with at least three geographic vertices.");
    const auto& c=in.camera;
    if (!(finite(c.focal_length_mm)&&c.focal_length_mm>0&&finite(c.pixel_pitch_um)&&c.pixel_pitch_um>0&&
          c.image_width_px>0&&c.image_height_px>0&&finite(c.target_gsd_cm_px)&&c.target_gsd_cm_px>0))
        return fail("Camera calibration and target GSD must be positive.");
    if (!(finite(c.forward_overlap_percent)&&c.forward_overlap_percent>=0&&c.forward_overlap_percent<90&&
          finite(c.side_overlap_percent)&&c.side_overlap_percent>=0&&c.side_overlap_percent<90))
        return fail("Forward and side overlap must be in [0,90) percent.");
    if (!(finite(in.minimum_altitude_agl_m)&&finite(in.maximum_altitude_agl_m)&&
          in.minimum_altitude_agl_m>=0&&in.maximum_altitude_agl_m>=in.minimum_altitude_agl_m&&
          finite(in.cruise_speed_mps)&&in.cruise_speed_mps>0&&finite(in.line_angle_deg)))
        return fail("Altitude bounds, cruise speed or sweep angle are invalid.");
    for (const auto& hole:in.exclusion_areas)
        if (!validRing(hole)) return fail("Each exclusion area must be a valid polygon ring.");

    const double gsd_m=c.target_gsd_cm_px/100.0;
    const double altitude=gsd_m*(c.focal_length_mm*1e-3)/(c.pixel_pitch_um*1e-6);
    if (altitude<in.minimum_altitude_agl_m || altitude>in.maximum_altitude_agl_m)
        return fail("Camera-derived AGL altitude is outside configured mission limits.");
    const double fw=gsd_m*c.image_width_px, fh=gsd_m*c.image_height_px;
    const double spacing=fw*(1.0-c.side_overlap_percent/100.0);
    if (!(finite(spacing)&&spacing>0.01)) return fail("Calculated coverage line spacing is invalid.");

    double lat0=0,lon0=0;
    for (const auto& p:in.area_of_interest){lat0+=p.latitude_deg;lon0+=p.longitude_deg;}
    lat0/=in.area_of_interest.size();lon0/=in.area_of_interest.size();
    if (std::abs(lat0)>85.0) return fail("AOI latitude is outside the supported local tangent approximation.");
    std::vector<XY> area; for(const auto& p:in.area_of_interest) area.push_back(project(p,lat0,lon0));
    std::vector<std::vector<XY>> holes;
    for(const auto& ring:in.exclusion_areas){std::vector<XY> h;for(const auto& p:ring)h.push_back(project(p,lat0,lon0));holes.push_back(std::move(h));}

    const double angle=in.line_angle_deg*kPi/180.0;
    // Rotate local coordinates so scan lines are horizontal in the working frame.
    auto rotate=[&](XY p){return XY{p.x*std::cos(angle)+p.y*std::sin(angle),-p.x*std::sin(angle)+p.y*std::cos(angle)};};
    auto unrotate=[&](XY p){return XY{p.x*std::cos(angle)-p.y*std::sin(angle),p.x*std::sin(angle)+p.y*std::cos(angle)};};
    for(auto& p:area)p=rotate(p);
    for(auto& h:holes)for(auto& p:h)p=rotate(p);
    double minY=std::numeric_limits<double>::max(),maxY=-minY;
    for(const auto& p:area){minY=std::min(minY,p.y);maxY=std::max(maxY,p.y);}
    const std::size_t lines=static_cast<std::size_t>(std::ceil((maxY-minY)/spacing));
    if(lines==0||lines>100000) return fail("AOI produces an empty or excessive number of coverage lines.");
    double length=0; bool reverse=false;
    for(std::size_t line=0;line<lines;++line){
        const double y=minY+spacing*(static_cast<double>(line)+0.5);
        auto spans=intervalsFor(area,y);
        for(const auto& h:holes){auto cuts=intervalsFor(h,y);spans=subtractIntervals(std::move(spans),cuts);}
        if(spans.empty())continue;
        if(reverse)std::reverse(spans.begin(),spans.end());
        for(auto span:spans){
            if(reverse)std::swap(span.lo,span.hi);
            XY a=unrotate({span.lo,y}),b=unrotate({span.hi,y});
            const double d=std::hypot(b.x-a.x,b.y-a.y);
            if(d<0.1)continue;
            if(!out.waypoints.empty()) length+=0; // transit turns are not yet modelled
            out.waypoints.push_back(unproject(a,lat0,lon0));
            out.waypoints.push_back(unproject(b,lat0,lon0));
            length+=d;
            reverse=!reverse;
        }
    }
    if(out.waypoints.size()<2)return fail("No usable coverage strips intersect the AOI after exclusions.");
    out.metrics.target_gsd_cm_px=c.target_gsd_cm_px;
    out.metrics.calculated_altitude_agl_m=altitude;
    out.metrics.footprint_width_m=fw;
    out.metrics.footprint_height_m=fh;
    out.metrics.line_spacing_m=spacing;
    out.metrics.estimated_track_length_m=length;
    out.metrics.estimated_flight_time_s=length/in.cruise_speed_mps;
    out.metrics.coverage_line_count=out.waypoints.size()/2;
    out.metrics.exclusion_gap_count=in.exclusion_areas.size();
    out.valid=true;
    out.status="CANDIDATE_2D_COVERAGE_ONLY";
    out.diagnostics.push_back("Candidate is a 2D sweep-line geometry; turn, terrain, obstacle, wind, energy and regulatory validation are not included.");
    out.diagnostics.push_back("Forward overlap is recorded as an input but along-track image triggering is not generated by this baseline.");
    return out;
}

} // namespace bluesky::planning
