#include "orchestrator/algorithm_orchestrator.hpp"
#include "../WEATHER/weather_candidate_assessor.hpp"
#include "../WEATHER/weather_field.hpp"
#include "../WEATHER/weather_trajectory_calculator.hpp"
#include <cassert>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>
using namespace bluesky::planning;
using namespace bluesky::weather;
namespace {
WindField fieldAt(int level,double east,double north,int validTime) {
    WindField f; f.level_agl_m=level; f.valid_date_utc=20261108; f.valid_time_utc=validTime;
    f.rows=2; f.columns=2; f.latitudes_deg={0.0,1.0}; f.longitudes_deg={10.0,11.0};
    f.eastward_mps={east,east,east,east}; f.northward_mps={north,north,north,north}; return f;
}
std::vector<WindField> forecast(double east,double north) {
    std::vector<WindField> f;
    for(int t:{600,700,800}) { f.push_back(fieldAt(50,east,north,t)); f.push_back(fieldAt(80,east,north,t)); }
    return f;
}
Route route(const std::string& id,bool eastbound) {
    Route r; r.lineage.route_id=id; r.lineage.route_version="1";
    r.lineage.mission_id="weather-rerank-mission"; r.lineage.mission_version="mission-v1";
    r.lineage.calculation_input_version="inputs-v1"; r.trajectory_geometry_complete=true;
    RouteWaypoint a; a.waypoint_id=id+"-START"; a.position={0.25,10.25};
    a.altitude_m=65.0; a.altitude_reference=AltitudeReference::AGL;
    RouteWaypoint b=a; b.waypoint_id=id+"-FINISH";
    b.position=eastbound?GeoPoint{0.25,10.26}:GeoPoint{0.26,10.25};
    r.waypoints={a,b}; r.segments={{"SEG",a.waypoint_id,b.waypoint_id,1112.0,20.0}};
    RouteSpeedProfile profile;
    profile.vehicle_model_id="TEST-UAV-MODEL";
    profile.vehicle_model_version="MODEL-V1";
    profile.configuration_id="cfg-1";
    profile.speed_reference="AIR_RELATIVE";
    profile.dynamics_source="CONTROLLED-TEST-LONGITUDINAL-DYNAMICS";
    profile.segments={{"SEG",1112.0,10.0,10.0,20.0,75.0,75.0,962.0,
        2.0,2.0,5.0,20.0}};
    r.speed_profile=profile;
    return r;
}
std::vector<CandidateSolution> candidates() {
    std::vector<CandidateSolution> out;
    for(const auto& item:{std::pair<std::string,bool>{"east-route",true},{"north-route",false}}) {
        CandidateSolution c; c.candidate_id=item.first; c.solver_id="weather-candidate-set";
        c.solver_version="1"; c.route_elements={"START","FINISH"}; c.estimated_time_s=80.0;
        c.estimated_energy_wh=20.0; c.estimated_reserve_wh=80.0; c.objective_score=80.0;
        c.feasibility=Feasibility::Feasible; c.trajectory_route=route(item.first,item.second);
        out.push_back(std::move(c));
    }
    return out;
}
class CandidateContext final:public SolverContext {
public:
    explicit CandidateContext(const MissionProblem& p):problem_(p){}
    const MissionProblem& problem()const override{return problem_;}
    const ComputeBudget& budget()const override{return budget_;}
    bool cancelled()const override{return false;}
    void publish(CandidateSolution c)override{candidates_.push_back(std::move(c));}
    const std::vector<CandidateSolution>& candidates()const override{return candidates_;}
private:
    const MissionProblem& problem_; ComputeBudget budget_{2000,256,8};
    std::vector<CandidateSolution> candidates_;
};
class CandidateSetSolver final:public Solver {
public:
    explicit CandidateSetSolver(std::vector<CandidateSolution> c):values_(std::move(c)){}
    SolverMetadata metadata()const override{return {"weather-candidate-set","1",{"point_to_point"},false,true};}
    bool eligible(const MissionProblem& p)const override{return p.problem_class=="point_to_point";}
    RunState run(SolverContext& c)override{for(const auto& v:values_)c.publish(v);return RunState::Completed;}
    void cancel()override{}
private: std::vector<CandidateSolution> values_;
};
OrchestratorDecision rank(const MissionProblem& p,const TrajectoryPerformanceInput& perf,
                          const std::vector<WindField>& fields,const std::string& snapshot) {
    std::vector<std::unique_ptr<Solver>> solvers;
    solvers.push_back(std::make_unique<CandidateSetSolver>(candidates()));
    CandidateContext context(p);
    auto assessor=makeCandidateWeatherAssessor(fields,perf,snapshot,"forecast-v1","weather-rerank-integration-v1");
    AlgorithmOrchestrator orchestrator(std::move(solvers),{},std::move(assessor));
    return orchestrator.solve(context);
}
}
int main() {
    MissionProblem p; p.mission_id="weather-rerank-mission"; p.mission_version="mission-v1";
    p.problem_class="point_to_point"; p.vehicle_ids={"uav-1"}; p.objective_priorities={"time"};
    TrajectoryPerformanceInput perf; perf.uav_id="uav-1"; perf.configuration_version="cfg-1";
    perf.performance_version="perf-1"; perf.mission_start_utc="2026-11-08T06:00:00Z";
    perf.cruise_airspeed_mps=15.0; perf.climb_rate_mps=3.0; perf.descent_rate_mps=3.0;
    perf.wind_tolerance_mps=30.0;
    perf.camera_configuration_id="CAMERA-TEST";
    perf.camera_gsd_m_px=0.02;
    perf.camera_forward_trigger_spacing_m=10.0;
    perf.camera_minimum_trigger_interval_s=0.5;
    perf.camera_exposure_time_s=0.001;
    perf.camera_maximum_motion_blur_px=10.0;
    perf.camera_capture_segment_ids={"SEG"};
    perf.energy_per_horizontal_meter_wh=0.001;
    perf.energy_per_climb_meter_wh=0.01; perf.energy_per_descent_meter_wh=0.01;
    perf.usable_energy_wh=100.0; perf.reserve_requirement_wh=10.0;
    const auto east=rank(p,perf,forecast(8.0,0.0),"GFS-EAST-FAVOURABLE");
    const auto north=rank(p,perf,forecast(0.0,8.0),"GFS-NORTH-FAVOURABLE");
    assert(east.feasibility==Feasibility::Feasible && north.feasibility==Feasibility::Feasible);
    assert(east.weather_assessment_applied && north.weather_assessment_applied);
    assert(east.weather_snapshot_id=="GFS-EAST-FAVOURABLE");
    assert(north.weather_snapshot_id=="GFS-NORTH-FAVOURABLE");
    assert(east.selected_candidate_id=="east-route");
    assert(north.selected_candidate_id=="north-route");
    assert(east.weather_corrected_time_s>0.0 && north.weather_corrected_time_s>0.0);
    bool eastCameraCapApplied=false;
    std::optional<Route> eastCorrectedRoute;
    for(const auto& candidate:east.admissible_candidates) {
        if(candidate.candidate_id!="east-route") continue;
        assert(candidate.trajectory_route.has_value());
        assert(candidate.trajectory_route->speed_profile.has_value());
        assert(candidate.trajectory_route->speed_profile->segments.size()==1);
        assert(candidate.trajectory_route->speed_profile->segments.front().maximum_reachable_speed_mps
               <=12.0+1e-6);
        assert(candidate.trajectory_route->speed_profile->segments.front().maximum_speed_mps==20.0);
        assert(candidate.trajectory_route->speed_profile->segments.front().applied_maximum_speed_mps
               <=12.0+1e-6);
        assert(candidate.trajectory_route->speed_profile->wind_snapshot_id=="GFS-EAST-FAVOURABLE");
        assert(candidate.trajectory_route->speed_profile->wind_snapshot_version=="forecast-v1");
        assert(candidate.trajectory_route->speed_profile->camera_configuration_id=="CAMERA-TEST");
        eastCorrectedRoute=candidate.trajectory_route;
        eastCameraCapApplied=true;
    }
    assert(eastCameraCapApplied);

    // A newer forecast must rebuild the derived profile from the retained
    // aircraft/geometry envelope, not inherit the previous tailwind cap.
    const auto refreshed=WeatherTrajectoryCalculator::evaluate(
        *eastCorrectedRoute,forecast(0.0,8.0),"GFS-REFRESHED","forecast-v2",
        perf,"weather-rerank-integration-v2");
    assert(refreshed.weather_ready);
    assert(refreshed.corrected_route.has_value());
    assert(refreshed.corrected_route->speed_profile.has_value());
    assert(refreshed.corrected_route->speed_profile->wind_snapshot_id=="GFS-REFRESHED");
    assert(refreshed.corrected_route->speed_profile->profile_version
           !=eastCorrectedRoute->speed_profile->profile_version);
    assert(refreshed.corrected_route->speed_profile->segments.front().maximum_reachable_speed_mps
           >eastCorrectedRoute->speed_profile->segments.front().maximum_reachable_speed_mps);
    assert(refreshed.corrected_route->speed_profile->segments.front().maximum_speed_mps==20.0);
    assert(refreshed.corrected_route->speed_profile->segments.front().applied_maximum_speed_mps==20.0);
    return 0;
}
