#include "algorithm_orchestrator.hpp"

#include <cassert>
#include <memory>
#include <string>
#include <utility>
#include <vector>

using namespace bluesky::planning;

namespace {
class TestContext final : public SolverContext {
public:
    explicit TestContext(const MissionProblem& problem) : problem_(problem) {}
    const MissionProblem& problem() const override { return problem_; }
    const ComputeBudget& budget() const override { return budget_; }
    bool cancelled() const override { return false; }
    void publish(CandidateSolution candidate) override {
        candidates_.push_back(std::move(candidate));
    }
    const std::vector<CandidateSolution>& candidates() const override {
        return candidates_;
    }
private:
    const MissionProblem& problem_;
    ComputeBudget budget_{2000, 256, 8};
    std::vector<CandidateSolution> candidates_;
};

Route completeRoute(const std::string& id)
{
    Route route;
    route.lineage.route_id = id;
    route.lineage.route_version = "1";
    route.trajectory_geometry_complete = true;
    RouteWaypoint a;
    a.waypoint_id = id + "-A";
    a.position = {55.0, 37.0};
    a.altitude_m = 100.0;
    a.altitude_reference = AltitudeReference::AGL;
    RouteWaypoint b = a;
    b.waypoint_id = id + "-B";
    b.position = {55.01, 37.01};
    route.waypoints = {a, b};
    RouteSegment segment;
    segment.segment_id = id + "-SEG";
    segment.from_waypoint_id = a.waypoint_id;
    segment.to_waypoint_id = b.waypoint_id;
    segment.distance_m = 1000.0;
    segment.nominal_speed_mps = 20.0;
    route.segments = {segment};
    return route;
}

class CandidateSolver final : public Solver {
public:
    SolverMetadata metadata() const override {
        return {"weather-test", "1", {"point_to_point"}, false, true};
    }
    bool eligible(const MissionProblem& problem) const override {
        return problem.problem_class == "point_to_point";
    }
    RunState run(SolverContext& context) override {
        const auto publish = [&](const std::string& id, double nominalTime,
                                 bool withGeometry) {
            CandidateSolution candidate;
            candidate.candidate_id = id;
            candidate.solver_id = "weather-test";
            candidate.solver_version = "1";
            candidate.route_elements = {"A", "B"};
            candidate.estimated_time_s = nominalTime;
            candidate.estimated_energy_wh = nominalTime;
            candidate.estimated_reserve_wh = 100.0 - nominalTime;
            candidate.objective_score = nominalTime;
            candidate.feasibility = Feasibility::Feasible;
            if (withGeometry) candidate.trajectory_route = completeRoute(id);
            context.publish(std::move(candidate));
        };
        publish("raw-fast", 10.0, true);
        publish("raw-slow", 20.0, true);
        publish("missing-geometry", 5.0, false);
        return RunState::Completed;
    }
    void cancel() override {}
};
} // namespace

int main()
{
    MissionProblem problem;
    problem.mission_id = "WEATHER-RANK-001";
    problem.problem_class = "point_to_point";
    problem.objective_priorities = {"time"};

    TestContext context(problem);
    std::vector<std::unique_ptr<Solver>> solvers;
    solvers.push_back(std::make_unique<CandidateSolver>());

    CandidateWeatherAssessor assessor =
        [](const MissionProblem&, const CandidateSolution& candidate) {
            CandidateWeatherAssessment result;
            result.evaluated = true;
            result.snapshot_id = "GFS-20261110-12";
            result.snapshot_version = "forecast-hour-006";
            if (candidate.candidate_id == "raw-fast") {
                result.corrected_time_s = 120.0;
                result.corrected_energy_wh = 55.0;
                result.remaining_energy_wh = 45.0;
            } else {
                result.corrected_time_s = 40.0;
                result.corrected_energy_wh = 25.0;
                result.remaining_energy_wh = 75.0;
            }
            return result;
        };

    AlgorithmOrchestrator orchestrator(std::move(solvers), {}, assessor);
    const auto decision = orchestrator.solve(context);

    assert(decision.feasibility == Feasibility::Feasible);
    assert(decision.selected_candidate_id == "raw-slow");
    assert(decision.weather_assessment_applied);
    assert(decision.weather_snapshot_id == "GFS-20261110-12");
    assert(decision.weather_corrected_time_s == 40.0);
    assert(decision.weather_corrected_energy_wh == 25.0);
    assert(decision.weather_remaining_energy_wh == 75.0);

    bool missingGeometryRejected = false;
    for (const auto& rejected : decision.rejected_candidates) {
        if (rejected.candidate_id != "missing-geometry") continue;
        for (const auto& violation : rejected.violations) {
            if (violation.find("WEATHER_ASSESSMENT_UNKNOWN") != std::string::npos)
                missingGeometryRejected = true;
        }
    }
    assert(missingGeometryRejected);
    return 0;
}
