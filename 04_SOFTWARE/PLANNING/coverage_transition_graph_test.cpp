#include "coverage_transition_graph.hpp"
#include <cassert>
#include <cmath>
#include <iostream>
using namespace bluesky::planning;
namespace {
GeoPoint p(double lat,double lon){return {lat,lon};}
CoverageTrack track(const char* id,GeoPoint start,GeoPoint end,std::size_t index){
    CoverageTrack t; t.generation_index=index; t.track_id=id; t.cell_id="MT01-CELL-0";
    t.start=start; t.end=end; t.altitude_m=100.0; t.length_m=10.0; return t;
}
ConstrainedEnvironmentSnapshot openEnvironment(){
    ConstrainedEnvironmentSnapshot e; e.snapshot_id="ENV-OPEN"; e.snapshot_version="1"; return e;
}
}
int main(){
    CoverageTrackResult tracks; tracks.valid=true; tracks.dependency_identity="TRACKS|1";
    tracks.tracks={
        track("MT01-TRACK-0",p(50.00000,8.00000),p(50.00000,8.00010),0),
        track("MT01-TRACK-1",p(50.00010,8.00010),p(50.00010,8.00000),1)};
    CoverageTransitionGraphInput input{tracks,openEnvironment(),"MT01-TRANSITION-1"};
    const auto result=CoverageTransitionGraphBuilder::build(input);
    assert(result.valid && result.track_ids.size()==2 && result.edges.size()==2 && result.rejected_edges==0);
    assert(result.edges[0].cost_m==result.edges[0].distance_m && result.edges[1].cost_m==result.edges[1].distance_m);
    const auto replay=CoverageTransitionGraphBuilder::build(input);
    assert(replay.valid && replay.dependency_identity==result.dependency_identity && replay.edges.size()==result.edges.size());
    for(std::size_t i=0;i<result.edges.size();++i){
        assert(replay.edges[i].from_track_id==result.edges[i].from_track_id);
        assert(replay.edges[i].to_track_id==result.edges[i].to_track_id);
        assert(std::abs(replay.edges[i].distance_m-result.edges[i].distance_m)<1e-9);
    }
    CoverageTransitionGraphInput blocked=input;
    SpatialRestriction r; r.active=true; r.restriction_id="R-TRANSITION"; r.source_id="TEST";
    r.geometry_type=RestrictionGeometryType::Circle; r.center=p(50.00005,8.00005); r.radius_m=20.0;
    r.minimum_altitude_m=0.0; r.maximum_altitude_m=200.0; blocked.environment.restrictions.push_back(r);
    const auto blockedResult=CoverageTransitionGraphBuilder::build(blocked);
    assert(blockedResult.valid && blockedResult.edges.empty() && blockedResult.rejected_edges==2);
    CoverageTransitionGraphInput invalid=input; invalid.tracks.valid=false;
    const auto invalidResult=CoverageTransitionGraphBuilder::build(invalid);
    assert(!invalidResult.valid && invalidResult.failure_code=="INVALID_TRACK_INPUT");
    std::cout<<"coverage_transition_graph_test: PASS\n";
    return 0;
}
