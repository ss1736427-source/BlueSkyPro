#include "grib2_decoder.hpp"
#include "grib2_weather_pipeline.hpp"
#include "weather_candidate_assessor.hpp"
#include "../PLANNING/orchestrator/algorithm_orchestrator.hpp"

#include <eccodes.h>

#include <cassert>
#include <cmath>
#include <cstdio>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace {
bool writeWindMessage(const char* path,const char* shortName,
                      double value,const char* mode,long forecastHour)
{
    codes_handle* handle=codes_handle_new_from_samples(nullptr,"regular_ll_sfc_grib2");
    if (!handle) return false;
    std::size_t nameLength=std::string(shortName).size();
    std::size_t levelTypeLength=std::string("heightAboveGround").size();
    const bool metadataOk=
        codes_set_string(handle,"shortName",shortName,&nameLength)==CODES_SUCCESS &&
        codes_set_string(handle,"typeOfLevel","heightAboveGround",&levelTypeLength)==CODES_SUCCESS &&
        codes_set_long(handle,"level",50)==CODES_SUCCESS &&
        codes_set_long(handle,"dataDate",20261108)==CODES_SUCCESS &&
        codes_set_long(handle,"dataTime",600)==CODES_SUCCESS &&
        codes_set_long(handle,"forecastTime",forecastHour)==CODES_SUCCESS;
    std::size_t count=0;
    bool valuesOk=metadataOk && codes_get_size(handle,"values",&count)==CODES_SUCCESS;
    std::vector<double> values(count,value);
    if (valuesOk)
        valuesOk=codes_set_double_array(handle,"values",values.data(),values.size())==CODES_SUCCESS;
    const bool writeOk=valuesOk && codes_write_message(handle,path,mode)==CODES_SUCCESS;
    codes_handle_delete(handle);
    return writeOk;
}

class FixtureContext final : public bluesky::planning::SolverContext {
public:
    explicit FixtureContext(const bluesky::planning::MissionProblem& problem)
        : problem_(problem) {}
    const bluesky::planning::MissionProblem& problem() const override { return problem_; }
    const bluesky::planning::ComputeBudget& budget() const override { return budget_; }
    bool cancelled() const override { return false; }
    void publish(bluesky::planning::CandidateSolution candidate) override {
        candidates_.push_back(std::move(candidate));
    }
    const std::vector<bluesky::planning::CandidateSolution>& candidates() const override {
        return candidates_;
    }
private:
    const bluesky::planning::MissionProblem& problem_;
    bluesky::planning::ComputeBudget budget_{2000,256,8};
    std::vector<bluesky::planning::CandidateSolution> candidates_;
};

class FixtureSolver final : public bluesky::planning::Solver {
public:
    explicit FixtureSolver(std::vector<bluesky::planning::CandidateSolution> candidates)
        : candidates_(std::move(candidates)) {}
    bluesky::planning::SolverMetadata metadata() const override {
        return {"grib2-fixture-solver","1",{"point_to_point"},false,true};
    }
    bool eligible(const bluesky::planning::MissionProblem& problem) const override {
        return problem.problem_class=="point_to_point";
    }
    bluesky::planning::RunState run(bluesky::planning::SolverContext& context) override {
        for (const auto& candidate:candidates_) context.publish(candidate);
        return bluesky::planning::RunState::Completed;
    }
    void cancel() override {}
private:
    std::vector<bluesky::planning::CandidateSolution> candidates_;
};

bluesky::planning::CandidateSolution makeCandidate(
    const std::string& id,const bluesky::planning::Route& route,double nominalTime)
{
    bluesky::planning::CandidateSolution candidate;
    candidate.candidate_id=id;
    candidate.solver_id="grib2-fixture-solver";
    candidate.solver_version="1";
    candidate.route_elements={"START","FINISH"};
    candidate.estimated_time_s=nominalTime;
    candidate.estimated_energy_wh=nominalTime;
    candidate.estimated_reserve_wh=1000.0-nominalTime;
    candidate.objective_score=nominalTime;
    candidate.feasibility=bluesky::planning::Feasibility::Feasible;
    candidate.trajectory_route=route;
    return candidate;
}
}

int main()
{
    const std::string pathF000="weather_decoder_fixture_f000.grib2";
    const std::string pathF001="weather_decoder_fixture_f001.grib2";
    std::remove(pathF000.c_str());
    std::remove(pathF001.c_str());

    assert(writeWindMessage(pathF000.c_str(),"u",4.0,"w",0));
    assert(writeWindMessage(pathF000.c_str(),"v",2.0,"a",0));
    assert(writeWindMessage(pathF001.c_str(),"u",6.0,"w",1));
    assert(writeWindMessage(pathF001.c_str(),"v",3.0,"a",1));

    std::vector<bluesky::weather::WindField> fields;
    std::string error;
    const bool decoded=bluesky::weather::Grib2Decoder::decodeWindFile(pathF000,fields,&error);
    assert(decoded && error.empty());
    assert(fields.size()==1);
    assert(fields[0].level_agl_m==50);
    assert(fields[0].valid_date_utc==20261108);
    assert(fields[0].valid_time_utc==600);
    assert(fields[0].rows>=2 && fields[0].columns>=2);
    assert(fields[0].eastward_mps.size()==fields[0].rows*fields[0].columns);
    assert(fields[0].northward_mps.size()==fields[0].rows*fields[0].columns);
    for (double value:fields[0].eastward_mps) assert(std::abs(value-4.0)<1e-6);
    for (double value:fields[0].northward_mps) assert(std::abs(value-2.0)<1e-6);

    {
        const auto& grid=fields.front();
        const std::size_t y=(grid.rows/2<grid.rows-1)?grid.rows/2:grid.rows-2;
        const std::size_t x=grid.columns/2;
        const double lat0=grid.latitudes_deg[y];
        const double lat1=lat0+0.00001;
        const double lon=grid.longitudes_deg[x];

        bluesky::planning::Route route;
        route.lineage.mission_id="fixture-mission";
        route.lineage.mission_version="1";
        route.lineage.route_id="fixture-route-north";
        route.lineage.route_version="1";
        route.waypoints={
            {"A",{lat0,lon},50.0,true,bluesky::planning::AltitudeReference::AGL},
            {"B",{lat1,lon},50.0,true,bluesky::planning::AltitudeReference::AGL}
        };
        route.segments={{"SEG-1","A","B",0.0,0.0}};
        route.trajectory_geometry_complete=true;

        auto southboundRoute=route;
        southboundRoute.lineage.route_id="fixture-route-south";
        std::swap(southboundRoute.waypoints[0],southboundRoute.waypoints[1]);
        southboundRoute.segments={{"SEG-2","B","A",0.0,0.0}};

        bluesky::planning::TrajectoryPerformanceInput input;
        input.uav_id="fixture-uav";
        input.mission_start_utc="2026-11-08T06:00:00Z";
        input.configuration_version="cfg-1";
        input.performance_version="perf-1";
        input.cruise_airspeed_mps=15.0;
        input.climb_rate_mps=3.0;
        input.descent_rate_mps=3.0;
        input.wind_tolerance_mps=30.0;
        input.energy_per_horizontal_meter_wh=0.0001;
        input.energy_per_climb_meter_wh=0.01;
        input.energy_per_descent_meter_wh=0.01;
        input.usable_energy_wh=1000.0;
        input.reserve_requirement_wh=10.0;

        const auto snapshot=bluesky::weather::Grib2WeatherPipeline::decodeFiles(
            {pathF000,pathF001},"fixture-snapshot","fixture-v1");
        assert(snapshot.decode_succeeded);
        assert(snapshot.error.empty());
        assert(snapshot.fields.size()==2);
        assert(snapshot.snapshot_id=="fixture-snapshot");
        assert(snapshot.snapshot_version=="fixture-v1");

        bluesky::planning::MissionProblem problem;
        problem.mission_id="fixture-mission";
        problem.mission_version="1";
        problem.problem_class="point_to_point";
        problem.vehicle_ids={"fixture-uav"};
        problem.objective_priorities={"time"};

        auto assessor=bluesky::weather::makeCandidateWeatherAssessor(
            snapshot.fields,input,snapshot.snapshot_id,snapshot.snapshot_version,
            "fixture-calc-v1");
        auto northCandidate=makeCandidate("northbound",route,110.0);
        auto southCandidate=makeCandidate("southbound",southboundRoute,100.0);
        const auto northAssessment=assessor(problem,northCandidate);
        const auto southAssessment=assessor(problem,southCandidate);
        assert(northAssessment.evaluated && northAssessment.error.empty());
        assert(southAssessment.evaluated && southAssessment.error.empty());
        assert(northAssessment.corrected_time_s>0.0);
        assert(southAssessment.corrected_time_s>0.0);
        // A northerly tailwind makes the northbound route faster than the
        // southbound headwind route. This proves decoded GRIB values affect
        // candidate ranking rather than merely producing a display result.
        assert(northAssessment.corrected_time_s<southAssessment.corrected_time_s);

        FixtureContext context(problem);
        std::vector<std::unique_ptr<bluesky::planning::Solver>> solvers;
        solvers.push_back(std::make_unique<FixtureSolver>(
            std::vector<bluesky::planning::CandidateSolution>{
                southCandidate,northCandidate}));
        bluesky::planning::AlgorithmOrchestrator orchestrator(
            std::move(solvers),{},assessor);
        const auto decision=orchestrator.solve(context);
        assert(decision.feasibility==bluesky::planning::Feasibility::Feasible);
        assert(decision.weather_assessment_applied);
        assert(decision.weather_snapshot_id=="fixture-snapshot");
        assert(decision.weather_snapshot_version=="fixture-v1");
        // Without weather the nominal 100 s southbound candidate wins;
        // after real GRIB2 decode and assessment the northbound candidate wins.
        assert(decision.selected_candidate_id=="northbound");
        assert(decision.weather_corrected_time_s==northAssessment.corrected_time_s);

        const auto pipeline=bluesky::weather::Grib2WeatherPipeline::evaluateFiles(
            {pathF000,pathF001},route,"fixture-snapshot","fixture-v1",input,"fixture-calc-v1");
        assert(pipeline.decode_succeeded);
        assert(pipeline.evaluation.weather_ready);
        assert(pipeline.evaluation.weather_error.empty());
        assert(pipeline.evaluation.trajectory.status==bluesky::planning::TrajectoryStatus::Feasible);
        assert(pipeline.evaluation.trajectory.wind_snapshot_id=="fixture-snapshot");

        const auto duplicate=bluesky::weather::Grib2WeatherPipeline::evaluateFiles(
            {pathF000,pathF000},route,"fixture-snapshot","fixture-v1",input,"fixture-calc-v1");
        assert(!duplicate.decode_succeeded);
        assert(duplicate.evaluation.weather_error.find("duplicate forecast field")!=std::string::npos);

        const auto single=bluesky::weather::Grib2WeatherPipeline::evaluateFile(
            pathF000,route,"fixture-snapshot","fixture-v1",input,"fixture-calc-v1");
        assert(single.decode_succeeded && single.evaluation.weather_ready);
    }

    fields.clear();
    assert(!bluesky::weather::Grib2Decoder::decodeWindFile(
        "this-file-does-not-exist.grib2",fields,&error));
    assert(fields.empty() && !error.empty());

    std::remove(pathF000.c_str());
    std::remove(pathF001.c_str());
    return 0;
}
