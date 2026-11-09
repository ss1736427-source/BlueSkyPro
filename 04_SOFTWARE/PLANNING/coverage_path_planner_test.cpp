#include "coverage_path_planner.hpp"
#include <cassert>
#include <cmath>
using namespace bluesky::planning;
static CoverageMissionInput input(){CoverageMissionInput x;x.mission_id="MT01-TEST";x.mission_version="1";x.area_of_interest={{0,0},{0,0.002},{0.002,0.002},{0.002,0}};x.camera={35,4,6000,4000,3,70,60};x.minimum_altitude_agl_m=10;x.maximum_altitude_agl_m=500;x.cruise_speed_mps=10;return x;}
int main(){
 auto in=input();auto r=CoveragePathPlanner::generate(in);assert(r.valid);assert(r.status=="CANDIDATE_2D_COVERAGE_ONLY");assert(r.waypoints.size()>=4);
 assert(r.metrics.coverage_line_count>0);assert(r.metrics.coverage_segment_count==r.waypoints.size()/2);
 assert(std::abs(r.metrics.line_spacing_m-r.metrics.footprint_width_m*0.4)<1e-6);
 assert(std::abs(r.metrics.forward_trigger_spacing_m-r.metrics.footprint_height_m*0.3)<1e-6);
 assert(std::abs(r.metrics.estimated_coverage_leg_time_s-r.metrics.estimated_coverage_leg_length_m/10)<1e-6);
 assert(!r.capture_points.empty());
 assert(r.capture_points.front().coverage_segment_index==0);
 assert(std::abs(r.capture_points.front().distance_from_segment_start_m)<1e-9);
 for(const auto& cp:r.capture_points){
  assert(cp.coverage_segment_index<r.metrics.coverage_segment_count);
  assert(cp.distance_from_segment_start_m>=0.0);
  assert(cp.distance_from_segment_start_m<=1000.0);
 }
 auto repeat=CoveragePathPlanner::generate(in);assert(repeat.valid&&repeat.waypoints.size()==r.waypoints.size());
 for(std::size_t i=0;i<r.waypoints.size();++i){assert(r.waypoints[i].latitude_deg==repeat.waypoints[i].latitude_deg);assert(r.waypoints[i].longitude_deg==repeat.waypoints[i].longitude_deg);}
 auto bad=in;bad.camera.side_overlap_percent=100;assert(!CoveragePathPlanner::generate(bad).valid);
 auto bow=in;bow.area_of_interest={{0,0},{0.002,0.002},{0,0.002},{0.002,0}};assert(!CoveragePathPlanner::generate(bow).valid);
 auto hole=in;hole.exclusion_areas={{{0.0005,0.0005},{0.0005,0.0015},{0.0015,0.0015},{0.0015,0.0005}}};
 auto h=CoveragePathPlanner::generate(hole);assert(h.valid);assert(h.metrics.exclusion_intersection_count>0);assert(h.metrics.coverage_segment_count>r.metrics.coverage_segment_count);
 auto badHole=in;badHole.exclusion_areas={{{0,0},{0.001,0.001},{0,0.001},{0.001,0}}};assert(!CoveragePathPlanner::generate(badHole).valid);
 auto closed=in;closed.area_of_interest.push_back(closed.area_of_interest.front());assert(CoveragePathPlanner::generate(closed).valid);
 auto outside=in;outside.exclusion_areas={{{0.0005,0.0005},{0.0005,0.003},{0.0015,0.003},{0.0015,0.0005}}};assert(!CoveragePathPlanner::generate(outside).valid);
 auto overlapping=in;overlapping.exclusion_areas={
  {{0.0004,0.0004},{0.0004,0.0012},{0.0012,0.0012},{0.0012,0.0004}},
  {{0.0008,0.0008},{0.0008,0.0016},{0.0016,0.0016},{0.0016,0.0008}}
 };assert(!CoveragePathPlanner::generate(overlapping).valid);
 auto concave=in;concave.area_of_interest={{0,0},{0,0.002},{0.0008,0.002},{0.0008,0.0008},{0.002,0.0008},{0.002,0}};assert(CoveragePathPlanner::generate(concave).valid);
 return 0;
}
