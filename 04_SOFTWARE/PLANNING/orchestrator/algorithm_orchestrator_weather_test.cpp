#include "algorithm_orchestrator.hpp"
#include "../notam_prohibited_zone_validator.hpp"

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
    route.environment.airspace_snapshot_id = "NOTAM-SNAPSHOT-WEATHER-TEST";
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
    segment.lateral_acceleration_mps2 = 1.0;
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
            if (withGeometry) {
                candidate.trajectory_route = completeRoute(id);
                attachFlightSafetyDynamics(candidate, *candidate.trajectory_route);
            }
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
    assert(decision.selected_route.has_value());
    assert(decision.selected_route->lineage.route_id == "raw-slow");
    assert(!decision.selected_flight_safety_validation.has_value());
    assert(!decision.selected_notam_validation.has_value());
    assert(decision.weather_assessment_applied);
    assert(decision.weather_snapshot_id == "GFS-20261110-12");
    assert(decision.weather_corrected_time_s == 40.0);
    assert(decision.weather_corrected_energy_wh == 25.0);
    assert(decision.weather_remaining_energy_wh == 75.0);
    assert(decision.admissible_candidates.size() == 2);
    bool retainedWeatherCorrectedWinner = false;
    for (const auto& admissible : decision.admissible_candidates) {
        if (admissible.candidate_id == "raw-slow") {
            retainedWeatherCorrectedWinner = true;
            assert(admissible.estimated_time_s == 40.0);
            assert(admissible.estimated_energy_wh == 25.0);
            assert(admissible.estimated_reserve_wh == 75.0);
        }
        assert(admissible.candidate_id != "missing-geometry");
    }
    assert(retainedWeatherCorrectedWinner);

    bool missingGeometryRejected = false;
    for (const auto& rejected : decision.rejected_candidates) {
        if (rejected.candidate_id != "missing-geometry") continue;
        for (const auto& violation : rejected.violations) {
            if (violation.find("WEATHER_ASSESSMENT_UNKNOWN") != std::string::npos)
                missingGeometryRejected = true;
        }
    }
    assert(missingGeometryRejected);

    // The same orchestrator must run the shared typed flight-safety validator before weather ranking.
    MissionProblem safetyProblem = problem;
    safetyProblem.vehicle_ids = {"TEST-UAV-001"};
    safetyProblem.flight_safety_vehicle_id = "TEST-UAV-001";
    safetyProblem.requires_flight_safety_validation = true;
    FlightSafetyEnvelope safetyEnvelope;
    safetyEnvelope.vehicle_model_id = "TEST-MODEL";
    safetyEnvelope.model_version = "MODEL-REV-1";
    safetyEnvelope.configuration_id = "TEST-CONFIG-REV-1";
    safetyEnvelope.source_reference = "CONTROLLED-TEST-EVIDENCE-1";
    safetyEnvelope.speed_reference = "AIR_RELATIVE";
    safetyEnvelope.minimum_speed_basis = "CONFIGURATION_SPECIFIC_MINIMUM_AIR_RELATIVE_SPEED";
    safetyEnvelope.evidence = FlightSafetyEvidence::OfficialValidated;
    safetyEnvelope.motion_class = MotionClass::Multirotor;
    safetyEnvelope.minimum_speed_mps = 5.0;
    safetyEnvelope.maximum_speed_mps = 30.0;
    safetyEnvelope.maximum_lateral_acceleration_mps2 = 2.0;
    safetyProblem.flight_safety_envelope = safetyEnvelope;
    TestContext safetyContext(safetyProblem);
    std::vector<std::unique_ptr<Solver>> safetySolvers;
    safetySolvers.push_back(std::make_unique<CandidateSolver>());
    AlgorithmOrchestrator safetyOrchestrator(std::move(safetySolvers), {}, assessor);
    const auto safetyDecision = safetyOrchestrator.solve(safetyContext);
    assert(safetyDecision.feasibility == Feasibility::Feasible);
    assert(safetyDecision.selected_flight_safety_validation.has_value());
    assert(safetyDecision.selected_flight_safety_validation->status == FlightSafetyStatus::Allowed);
    assert(safetyDecision.selected_flight_safety_validation->configuration_id == "TEST-CONFIG-REV-1");

    // Missing envelope or incomplete segment dynamics must not produce a selected candidate.
    MissionProblem unknownSafetyProblem = safetyProblem;
    unknownSafetyProblem.flight_safety_envelope.reset();
    TestContext unknownSafetyContext(unknownSafetyProblem);
    std::vector<std::unique_ptr<Solver>> unknownSafetySolvers;
    unknownSafetySolvers.push_back(std::make_unique<CandidateSolver>());
    AlgorithmOrchestrator unknownSafetyOrchestrator(std::move(unknownSafetySolvers), {}, assessor);
    const auto unknownSafetyDecision = unknownSafetyOrchestrator.solve(unknownSafetyContext);
    assert(!unknownSafetyDecision.selected_flight_safety_validation.has_value());
    assert(unknownSafetyDecision.feasibility != Feasibility::Feasible);

    // Final NOTAM validation runs after the weather assessor returns the matching 4D trajectory.
    MissionProblem notamProblem = problem;
    notamProblem.requires_notam_validation = true;
    auto snapshot = std::make_shared<NotamSnapshot>();
    snapshot->snapshot_id = "NOTAM-SNAPSHOT-WEATHER-TEST";
    snapshot->source_id = "CONTROLLED-TEST-SOURCE";
    snapshot->source_version = "1";
    snapshot->evaluation_epoch_s = 1000;
    snapshot->planned_mission_start_epoch_s = 1000;
    snapshot->planned_mission_end_epoch_s = 1100;
    NotamZone crossingZone;
    crossingZone.notam_id = "NOTAM-TIMED-001";
    crossingZone.component_id = "NOTAM-TIMED-001-COMP-01";
    crossingZone.geometry_type = NotamGeometryType::Circle;
    crossingZone.center = {55.005, 37.005};
    crossingZone.radius_m = 150.0;
    crossingZone.lower_altitude_m = 90.0;
    crossingZone.upper_altitude_m = 110.0;
    crossingZone.valid_from_epoch_s = 1040;
    crossingZone.valid_to_epoch_s = 1060;
    crossingZone.altitude_reference = NotamAltitudeReference::AGL;
    snapshot->zones.push_back(crossingZone);
    notamProblem.notam_snapshot = snapshot;

    TestContext notamContext(notamProblem);
    std::vector<std::unique_ptr<Solver>> notamSolvers;
    notamSolvers.push_back(std::make_unique<CandidateSolver>());
    CandidateWeatherAssessor timedAssessor =
        [](const MissionProblem&, const CandidateSolution& candidate) {
            CandidateWeatherAssessment result;
            result.evaluated = true;
            result.snapshot_id = "GFS-TEST";
            result.snapshot_version = "1";
            result.corrected_time_s = 100.0;
            result.corrected_energy_wh = 20.0;
            result.remaining_energy_wh = 80.0;
            const auto& route = *candidate.trajectory_route;
            TrajectoryResult trajectory;
            trajectory.status = TrajectoryStatus::Feasible;
            trajectory.route_id = route.lineage.route_id;
            trajectory.route_version = route.lineage.route_version;
            trajectory.mission_start_epoch_s = 1000;
            trajectory.total_time_s = 100.0;
            trajectory.total_energy_wh = 20.0;
            trajectory.remaining_energy_wh = 80.0;
            for (std::size_t i = 0; i < route.waypoints.size(); ++i) {
                const auto& waypoint = route.waypoints[i];
                trajectory.points.push_back({
                    waypoint.waypoint_id, waypoint.position, waypoint.altitude_m,
                    i == 0 ? 0.0 : 100.0, 0.0
                });
            }
            trajectory.segments.push_back({
                route.segments.front().segment_id, 10.0, 45.0, 100.0, 20.0
            });
            result.trajectory = trajectory;
            return result;
        };
    AlgorithmOrchestrator notamOrchestrator(
        std::move(notamSolvers), {}, timedAssessor);
    const auto notamDecision = notamOrchestrator.solve(notamContext);
    assert(notamDecision.feasibility == Feasibility::Infeasible);
    bool temporalNotamConflictRejected = false;
    for (const auto& rejected : notamDecision.rejected_candidates) {
        for (const auto& violation : rejected.violations) {
            if (violation.find("NOTAM_VALIDATION") != std::string::npos &&
                violation.find("NOTAM-TIMED-001") != std::string::npos &&
                violation.find("NOTAM-TIMED-001-COMP-01") != std::string::npos)
                temporalNotamConflictRejected = true;
        }
    }
    assert(temporalNotamConflictRejected);

    // A clear, time-valid NOTAM snapshot retains the exact successful 4D validation result.
    MissionProblem clearNotamProblem = problem;
    clearNotamProblem.requires_notam_validation = true;
    auto clearSnapshot = std::make_shared<NotamSnapshot>(*snapshot);
    clearSnapshot->zones.clear();
    clearNotamProblem.notam_snapshot = clearSnapshot;
    TestContext clearNotamContext(clearNotamProblem);
    std::vector<std::unique_ptr<Solver>> clearNotamSolvers;
    clearNotamSolvers.push_back(std::make_unique<CandidateSolver>());
    AlgorithmOrchestrator clearNotamOrchestrator(
        std::move(clearNotamSolvers), {}, timedAssessor);
    const auto clearNotamDecision = clearNotamOrchestrator.solve(clearNotamContext);
    assert(clearNotamDecision.feasibility == Feasibility::Feasible);
    assert(clearNotamDecision.selected_notam_validation.has_value());
    assert(clearNotamDecision.selected_notam_validation->status == RouteValidationStatus::Allowed);
    assert(clearNotamDecision.selected_notam_validation->snapshot_id == clearSnapshot->snapshot_id);
    assert(clearNotamDecision.selected_weather_trajectory.has_value());
    assert(clearNotamDecision.selected_route.has_value());

    MissionProblem missingSnapshotProblem = problem;
    missingSnapshotProblem.requires_notam_validation = true;
    TestContext missingSnapshotContext(missingSnapshotProblem);
    std::vector<std::unique_ptr<Solver>> missingSnapshotSolvers;
    missingSnapshotSolvers.push_back(std::make_unique<CandidateSolver>());
    AlgorithmOrchestrator missingSnapshotOrchestrator(
        std::move(missingSnapshotSolvers), {}, assessor);
    assert(missingSnapshotOrchestrator.solve(missingSnapshotContext).feasibility ==
           Feasibility::Uncertain);

    return 0;
}
