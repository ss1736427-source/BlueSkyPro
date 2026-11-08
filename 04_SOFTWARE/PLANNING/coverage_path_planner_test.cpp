#include "coverage_path_planner.hpp"
#include <cassert>
#include <cmath>
using namespace bluesky::planning;

int main() {
    CoverageMissionInput in;
    in.mission_id="MT01-TEST"; in.mission_version="1";
    in.area_of_interest={{0.0,0.0},{0.0,0.002},{0.002,0.002},{0.002,0.0}};
    in.camera={35.0,4.0,6000,4000,3.0,70.0,60.0};
    in.minimum_altitude_agl_m=10; in.maximum_altitude_agl_m=500;
    in.cruise_speed_mps=10;
    auto result=CoveragePathPlanner::generate(in);
    assert(result.valid);
    assert(result.status=="CANDIDATE_2D_COVERAGE_ONLY");
    assert(result.waypoints.size()>=4);
    assert(result.metrics.coverage_line_count>0);
    assert(std::abs(result.metrics.line_spacing_m-
        result.metrics.footprint_width_m*0.4)<1e-6);

    auto bad=in; bad.camera.side_overlap_percent=100;
    assert(!CoveragePathPlanner::generate(bad).valid);

    auto excluded=in;
    excluded.exclusion_areas={{{0.0005,0.0005},{0.0005,0.0015},
                               {0.0015,0.0015},{0.0015,0.0005}}};
    auto withHole=CoveragePathPlanner::generate(excluded);
    assert(withHole.valid);
    assert(withHole.metrics.exclusion_gap_count==1);
    return 0;
}
