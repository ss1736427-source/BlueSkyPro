#include "coverage_transition_graph.hpp"
#include <cmath>
namespace bluesky::planning {
namespace {
constexpr double kPi=3.14159265358979323846;
constexpr double kMetersPerDegree=111320.0;
double distance(const GeoPoint& a,const GeoPoint& b){
    const double lat=(a.latitude_deg+b.latitude_deg)*0.5*kPi/180.0;
    const double dx=(b.longitude_deg-a.longitude_deg)*kMetersPerDegree*std::cos(lat);
    const double dy=(b.latitude_deg-a.latitude_deg)*kMetersPerDegree;
    return std::sqrt(dx*dx+dy*dy);
}
}
CoverageTransitionGraphResult CoverageTransitionGraphBuilder::build(const CoverageTransitionGraphInput& input){
    CoverageTransitionGraphResult result;
    if(!input.tracks.valid){result.failure_code="INVALID_TRACK_INPUT";return result;}
    if(input.tracks.tracks.empty()){result.failure_code="NO_COVERAGE_TRACK";return result;}
    result.dependency_identity=input.tracks.dependency_identity+"|"+input.calculation_version;
    for(const auto& track:input.tracks.tracks){
        if(track.track_id.empty() ||
           !std::isfinite(track.start.latitude_deg) || !std::isfinite(track.start.longitude_deg) ||
           !std::isfinite(track.end.latitude_deg) || !std::isfinite(track.end.longitude_deg) ||
           !std::isfinite(track.altitude_m) || !(track.length_m>0.0) || !std::isfinite(track.length_m)){
            result.failure_code="INVALID_COVERAGE_TRACK";return result;
        }
        result.track_ids.push_back(track.track_id);
    }
    // Controlled first slice: explicit DISTANCE_ONLY cost; no hidden weights.
    for(const auto& from:input.tracks.tracks){
        for(const auto& to:input.tracks.tracks){
            if(from.track_id==to.track_id) continue;
            if(std::abs(from.altitude_m-to.altitude_m)>1e-9){++result.rejected_edges;continue;}
            const auto check=ConstrainedOpenSpace::evaluateSegment(
                input.environment,{from.end,to.start,from.altitude_m,from.altitude_m,from.altitude_m});
            if(!check.allowed){++result.rejected_edges;continue;}
            const double d=distance(from.end,to.start);
            if(!(std::isfinite(d)&&d>=0.0)){result.failure_code="INVALID_TRANSITION_DISTANCE";return result;}
            result.edges.push_back({from.track_id,to.track_id,d,d});
        }
    }
    result.valid=true;
    return result;
}
} // namespace bluesky::planning
