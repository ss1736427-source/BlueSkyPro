#include "coverage_path_planner.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
#include <utility>
namespace bluesky::planning { namespace {
constexpr double kPi=3.14159265358979323846, kEarthRadiusM=6371008.8, kEps=1e-7;
struct XY{double x,y;}; struct Interval{double lo,hi;};
bool finite(double v){return std::isfinite(v);}
double cross(XY a,XY b,XY c){return (b.x-a.x)*(c.y-a.y)-(b.y-a.y)*(c.x-a.x);}
bool onSegment(XY a,XY b,XY p){return std::abs(cross(a,b,p))<=kEps&&p.x>=std::min(a.x,b.x)-kEps&&p.x<=std::max(a.x,b.x)+kEps&&p.y>=std::min(a.y,b.y)-kEps&&p.y<=std::max(a.y,b.y)+kEps;}
bool intersects(XY a,XY b,XY c,XY d){
 double x=cross(a,b,c),y=cross(a,b,d),z=cross(c,d,a),w=cross(c,d,b);
 if(((x>kEps&&y< -kEps)||(x< -kEps&&y>kEps))&&((z>kEps&&w< -kEps)||(z< -kEps&&w>kEps)))return true;
 return onSegment(a,b,c)||onSegment(a,b,d)||onSegment(c,d,a)||onSegment(c,d,b);
}
bool validRing(const std::vector<XY>& r){
 if(r.size()<3)return false; double area=0;
 for(std::size_t i=0;i<r.size();++i){auto a=r[i],b=r[(i+1)%r.size()];if(std::hypot(a.x-b.x,a.y-b.y)<0.01)return false;area+=a.x*b.y-b.x*a.y;}
 if(std::abs(area)<0.02)return false;
 for(std::size_t i=0;i<r.size();++i)for(std::size_t j=i+1;j<r.size();++j){
  std::size_t i2=(i+1)%r.size(),j2=(j+1)%r.size(); if(i==j||i2==j||j2==i)continue;
  if(intersects(r[i],r[i2],r[j],r[j2]))return false;
 } return true;
}
bool validGeo(const std::vector<GeoPoint>& r){
 if(r.size()<3)return false; for(const auto&p:r)if(!finite(p.latitude_deg)||!finite(p.longitude_deg)||p.latitude_deg< -90||p.latitude_deg>90||p.longitude_deg< -180||p.longitude_deg>180)return false; return true;
}
void normalizeRing(std::vector<XY>& r){
 if(r.size()>3&&std::hypot(r.front().x-r.back().x,r.front().y-r.back().y)<0.01)r.pop_back();
}
XY project(const GeoPoint&p,double lat,double lon){double c=std::cos(lat*kPi/180);return{kEarthRadiusM*(p.longitude_deg-lon)*kPi/180*c,kEarthRadiusM*(p.latitude_deg-lat)*kPi/180};}
GeoPoint unproject(XY p,double lat,double lon){double c=std::cos(lat*kPi/180);return{lat+p.y/kEarthRadiusM*180/kPi,lon+p.x/(kEarthRadiusM*c)*180/kPi};}
std::vector<double> intersectionsAt(const std::vector<XY>&r,double y){
 std::vector<double>x;for(std::size_t i=0,j=r.size()-1;i<r.size();j=i++){auto a=r[j],b=r[i];if((a.y<=y&&b.y>y)||(b.y<=y&&a.y>y))x.push_back(a.x+(y-a.y)*(b.x-a.x)/(b.y-a.y));}std::sort(x.begin(),x.end());return x;
}
std::vector<Interval> spansAt(const std::vector<XY>&r,double y){auto x=intersectionsAt(r,y);std::vector<Interval>s;for(std::size_t i=1;i<x.size();i+=2)if(x[i]-x[i-1]>0.01)s.push_back({x[i-1],x[i]});return s;}
std::vector<Interval> mergeIntervals(std::vector<Interval> values){
 std::sort(values.begin(),values.end(),[](const Interval&a,const Interval&b){return a.lo<b.lo;});
 std::vector<Interval> merged;
 for(const auto& value:values){
  if(value.hi-value.lo<=0.01)continue;
  if(merged.empty()||value.lo>merged.back().hi+0.01)merged.push_back(value);
  else merged.back().hi=std::max(merged.back().hi,value.hi);
 }
 return merged;
}
std::vector<Interval> subtract(std::vector<Interval>b,const std::vector<Interval>&cuts){
 for(auto c:cuts){std::vector<Interval>n;for(auto s:b){if(c.hi<=s.lo||c.lo>=s.hi){n.push_back(s);continue;}if(c.lo>s.lo)n.push_back({s.lo,std::min(c.lo,s.hi)});if(c.hi<s.hi)n.push_back({std::max(c.hi,s.lo),s.hi});}b=std::move(n);}return b;
}
}
CoveragePlanResult CoveragePathPlanner::generate(const CoverageMissionInput&in){
 CoveragePlanResult out;auto fail=[&](const std::string&m){out.valid=false;out.status="INVALID_INPUT";out.diagnostics.push_back(m);return out;};
 if(!validGeo(in.area_of_interest))return fail("AOI requires at least three finite geographic vertices.");
 const auto&c=in.camera;
 if(!(finite(c.focal_length_mm)&&c.focal_length_mm>0&&finite(c.pixel_pitch_um)&&c.pixel_pitch_um>0&&c.image_width_px&&c.image_height_px&&finite(c.target_gsd_cm_px)&&c.target_gsd_cm_px>0))return fail("Camera calibration and target GSD must be positive.");
 if(c.intrinsic_calibration_verified&&(c.camera_configuration_id.empty()||c.calibration_source_reference.empty()))return fail("Verified camera intrinsics require an exact camera configuration and calibration source reference.");
 if(!(finite(c.forward_overlap_percent)&&c.forward_overlap_percent>=0&&c.forward_overlap_percent<90&&finite(c.side_overlap_percent)&&c.side_overlap_percent>=0&&c.side_overlap_percent<90))return fail("Forward and side overlap must be in [0,90) percent.");
 if(!(finite(in.minimum_altitude_agl_m)&&finite(in.maximum_altitude_agl_m)&&in.minimum_altitude_agl_m>=0&&in.maximum_altitude_agl_m>=in.minimum_altitude_agl_m&&finite(in.cruise_speed_mps)&&in.cruise_speed_mps>0&&finite(in.line_angle_deg)))return fail("Altitude bounds, cruise speed or sweep angle are invalid.");
 double lat=0,lon=0;for(auto p:in.area_of_interest){lat+=p.latitude_deg;lon+=p.longitude_deg;}lat/=in.area_of_interest.size();lon/=in.area_of_interest.size();
 if(std::abs(lat)>85)return fail("AOI latitude is outside supported local projection range.");
 std::vector<XY>area;for(auto p:in.area_of_interest)area.push_back(project(p,lat,lon));normalizeRing(area);
 if(!validRing(area))return fail("AOI ring is degenerate or self-intersecting.");
 double minX=std::numeric_limits<double>::max(),maxX=-minX,minY=minX,maxY=-minX;
 for(auto p:area){minX=std::min(minX,p.x);maxX=std::max(maxX,p.x);minY=std::min(minY,p.y);maxY=std::max(maxY,p.y);}
 if(maxX-minX>50000||maxY-minY>50000)return fail("AOI exceeds the 50 km local-projection extent limit.");
 std::vector<std::vector<XY>>holes;
 for(const auto&r:in.exclusion_areas){if(!validGeo(r))return fail("Exclusion polygon requires at least three finite vertices.");std::vector<XY>h;for(auto p:r)h.push_back(project(p,lat,lon));normalizeRing(h);
  if(!validRing(h))return fail("An exclusion ring is degenerate or self-intersecting.");
  // Exclusions are intersected with AOI scanline spans below. This naturally
  // clips boundary-crossing zones and unions overlapping/contained zones without
  // constructing artificial polygon bridges or rejecting valid restriction sets.
  holes.push_back(std::move(h));
 }
 double gsd=c.target_gsd_cm_px/100,alt=gsd*(c.focal_length_mm*1e-3)/(c.pixel_pitch_um*1e-6);
 if(alt<in.minimum_altitude_agl_m||alt>in.maximum_altitude_agl_m)return fail("Camera-derived AGL altitude is outside mission limits.");
 if(!finite(c.minimum_trigger_interval_s)||c.minimum_trigger_interval_s<0.0)return fail("Camera minimum trigger interval must be finite and non-negative.");
 if(!finite(c.exposure_time_s)||c.exposure_time_s<0.0||!finite(c.maximum_motion_blur_px)||c.maximum_motion_blur_px<0.0)return fail("Camera exposure and motion-blur tolerance must be finite and non-negative.");
 const bool triggerLimitConfigured=c.minimum_trigger_interval_s>0.0;
 const bool blurLimitConfigured=c.exposure_time_s>0.0||c.maximum_motion_blur_px>0.0;
 if(blurLimitConfigured&&(!finite(c.exposure_time_s)||c.exposure_time_s<=0.0||!finite(c.maximum_motion_blur_px)||c.maximum_motion_blur_px<=0.0))return fail("Motion-blur validation requires a positive selected exposure time and maximum blur tolerance.");
 double fw=gsd*c.image_width_px,fh=gsd*c.image_height_px,spacing=fw*(1-c.side_overlap_percent/100),trigger=fh*(1-c.forward_overlap_percent/100);
 if(!(finite(spacing)&&spacing>0.01&&finite(trigger)&&trigger>0.01))return fail("Calculated coverage or trigger spacing is invalid.");
 const double nominalTriggerInterval=trigger/in.cruise_speed_mps;
 const double nominalMotionBlurPx=blurLimitConfigured?in.cruise_speed_mps*c.exposure_time_s/gsd:0.0;
 double maximumCameraGroundSpeed=std::numeric_limits<double>::infinity();
 if(triggerLimitConfigured)maximumCameraGroundSpeed=std::min(maximumCameraGroundSpeed,trigger/c.minimum_trigger_interval_s);
 if(blurLimitConfigured)maximumCameraGroundSpeed=std::min(maximumCameraGroundSpeed,c.maximum_motion_blur_px*gsd/c.exposure_time_s);
 if(triggerLimitConfigured&&nominalTriggerInterval+1e-9<c.minimum_trigger_interval_s)return fail("Required image spacing exceeds the camera's supported trigger rate at the planned ground speed.");
 if(blurLimitConfigured&&nominalMotionBlurPx>c.maximum_motion_blur_px+1e-9)return fail("Planned ground speed and selected exposure exceed the configured motion-blur tolerance.");
 double angle=std::remainder(in.line_angle_deg,360.0)*kPi/180;
 auto rot=[&](XY p){return XY{p.x*std::cos(angle)+p.y*std::sin(angle),-p.x*std::sin(angle)+p.y*std::cos(angle)};};
 auto unrot=[&](XY p){return XY{p.x*std::cos(angle)-p.y*std::sin(angle),p.x*std::sin(angle)+p.y*std::cos(angle)};};
 for(auto&p:area)p=rot(p);for(auto&h:holes)for(auto&p:h)p=rot(p);
 minY=std::numeric_limits<double>::max();maxY=-minY;for(auto p:area){minY=std::min(minY,p.y);maxY=std::max(maxY,p.y);}
 std::size_t possible=static_cast<std::size_t>(std::ceil((maxY-minY)/spacing));if(!possible||possible>100000)return fail("AOI produces an empty or excessive number of coverage lines.");
 double length=0;std::size_t lines=0,segments=0,gaps=0;
 for(std::size_t line=0;line<possible;++line){
  double y=minY+spacing*(double(line)+0.5);auto spans=spansAt(area,y);
  std::vector<Interval> cuts;
  for(const auto&h:holes){
   const auto zoneSpans=spansAt(h,y);
   for(const auto&base:spans)for(const auto&zone:zoneSpans){
    const double lo=std::max(base.lo,zone.lo),hi=std::min(base.hi,zone.hi);
    if(hi-lo>0.01)cuts.push_back({lo,hi});
   }
  }
  cuts=mergeIntervals(std::move(cuts));
  if(!cuts.empty())++gaps;
  spans=subtract(std::move(spans),cuts);
  spans.erase(std::remove_if(spans.begin(),spans.end(),[](auto s){return s.hi-s.lo<0.1;}),spans.end());if(spans.empty())continue;
  if(line%2)std::reverse(spans.begin(),spans.end());++lines;
  for(auto s:spans){
   if(line%2)std::swap(s.lo,s.hi);
   XY a=unrot({s.lo,y}),b=unrot({s.hi,y});
   const double segmentLength=std::hypot(b.x-a.x,b.y-a.y);
   const double triggerCount=std::floor(segmentLength/trigger)+1.0;
   if(!finite(triggerCount)||triggerCount<1.0||out.capture_points.size()+triggerCount>2000000.0)
    return fail("Camera trigger output exceeds the 2,000,000 point safety limit.");
   out.waypoints.push_back(unproject(a,lat,lon));
   out.waypoints.push_back(unproject(b,lat,lon));
   double lastCaptureDistance=-1.0;
   for(std::size_t k=0;k<static_cast<std::size_t>(triggerCount);++k){
    const double d=std::min(double(k)*trigger,segmentLength);
    const double ratio=segmentLength>0.0?d/segmentLength:0.0;
    XY point{a.x+(b.x-a.x)*ratio,a.y+(b.y-a.y)*ratio};
    out.capture_points.push_back({unproject(point,lat,lon),segments,d});
    lastCaptureDistance=d;
   }
   // Include the segment endpoint when the remaining gap is non-zero so the
   // final part of every coverage leg is not silently left without a capture station.
   if(segmentLength-lastCaptureDistance>0.01)
    out.capture_points.push_back({unproject(b,lat,lon),segments,segmentLength});
   length+=segmentLength;++segments;
  }
 }
 if(out.waypoints.size()<2)return fail("No usable coverage strips intersect the AOI after exclusions.");
 out.metrics.target_gsd_cm_px=c.target_gsd_cm_px;out.metrics.calculated_altitude_agl_m=alt;out.metrics.footprint_width_m=fw;out.metrics.footprint_height_m=fh;out.metrics.line_spacing_m=spacing;out.metrics.forward_trigger_spacing_m=trigger;out.metrics.estimated_coverage_leg_length_m=length;out.metrics.estimated_coverage_leg_time_s=length/in.cruise_speed_mps;out.metrics.nominal_trigger_interval_s=nominalTriggerInterval;out.metrics.nominal_motion_blur_px=nominalMotionBlurPx;out.metrics.maximum_camera_ground_speed_mps=finite(maximumCameraGroundSpeed)?maximumCameraGroundSpeed:0.0;out.metrics.coverage_line_count=lines;out.metrics.coverage_segment_count=segments;out.metrics.exclusion_intersection_count=gaps;
 out.valid=true;out.status="CANDIDATE_2D_COVERAGE_ONLY";out.intrinsic_calibration_verified=c.intrinsic_calibration_verified;
 out.diagnostics.push_back("Coverage-leg length/time exclude transitions, turns and ferry legs; they are not total mission distance or flight time.");
 out.diagnostics.push_back("Capture points are emitted along straight coverage legs; image_height_px is assumed to be the along-track sensor dimension. Trigger timing is derived from spacing / planned ground speed; actual wind-corrected timing must be re-evaluated by the trajectory stage.");
 if(!triggerLimitConfigured||!blurLimitConfigured)out.diagnostics.push_back("CAMERA_CAPTURE_QUALITY_UNVERIFIED: hardware trigger interval, selected shutter exposure and motion-blur tolerance must be supplied before flight release.");
 else out.diagnostics.push_back("Camera trigger-rate and motion-blur limits are checked at nominal planned ground speed; the wind-corrected trajectory must recheck actual segment ground speed.");
 if(!c.intrinsic_calibration_verified)out.diagnostics.push_back("Camera optical geometry is reference-only; individual camera/lens intrinsics and distortion calibration are not verified.");
 out.diagnostics.push_back("No DEM following, turn kinematics, obstacle routing, wind/energy model, coverage-union proof or regulatory validation.");
 out.diagnostics.push_back("Local equirectangular projection is limited to AOIs no larger than 50 km in either projected dimension.");
 return out;
}
} // namespace bluesky::planning
