#include "weather_candidate_assessor.hpp"

#include "weather_trajectory_calculator.hpp"

#include <cmath>
#include <memory>
#include <sstream>
#include <utility>

namespace bluesky::weather {

bluesky::planning::CandidateWeatherAssessor makeCandidateWeatherAssessor(
    std::vector<WindField> fields,
    bluesky::planning::TrajectoryPerformanceInput performance,
    std::string snapshot_id,
    std::string snapshot_version,
    std::string calculation_version,
    int maximum_vertical_gap_m)
{
    using namespace bluesky::planning;
    auto sharedFields = std::make_shared<const std::vector<WindField>>(std::move(fields));
    return [sharedFields, performance = std::move(performance),
            snapshot_id = std::move(snapshot_id),
            snapshot_version = std::move(snapshot_version),
            calculation_version = std::move(calculation_version),
            maximum_vertical_gap_m](
                const MissionProblem& problem,
                const CandidateSolution& candidate) -> CandidateWeatherAssessment {
        CandidateWeatherAssessment result;
        if (problem.vehicle_ids.size() != 1
            || problem.vehicle_ids.front().empty()
            || performance.uav_id != problem.vehicle_ids.front()) {
            result.error = "weather assessment requires one assigned UAV matching the validated performance profile";
            return result;
        }
        if (!candidate.trajectory_route.has_value()) {
            result.error = "candidate has no canonical trajectory geometry";
            return result;
        }
        const Route& route = *candidate.trajectory_route;
        if (!route.trajectory_geometry_complete
            || route.waypoints.size() < 2
            || route.segments.size() + 1 != route.waypoints.size()) {
            result.error = "candidate trajectory is incomplete, including transitions or turns";
            return result;
        }
        if (route.lineage.mission_id != problem.mission_id
            || (!problem.mission_version.empty()
                && route.lineage.mission_version != problem.mission_version)) {
            result.error = "candidate route lineage does not match the current mission version";
            return result;
        }
        if (snapshot_id.empty() || snapshot_version.empty()
            || calculation_version.empty() || maximum_vertical_gap_m <= 0
            || sharedFields->empty()) {
            result.error = "decoded forecast snapshot, calculation version and vertical policy are required";
            return result;
        }
        if (!route.environment.wind_snapshot_id.empty()
            && route.environment.wind_snapshot_id != snapshot_id) {
            result.error = "candidate route references a different wind snapshot";
            return result;
        }

        auto input = performance;
        input.wind_snapshot_id = snapshot_id;
        input.wind_snapshot_version = snapshot_version;
        const auto evaluation = WeatherTrajectoryCalculator::evaluate(
            route, *sharedFields, snapshot_id, snapshot_version,
            std::move(input), calculation_version, maximum_vertical_gap_m);
        if (!evaluation.weather_ready
            || evaluation.trajectory.status != TrajectoryStatus::Feasible) {
            result.error = evaluation.weather_error;
            if (result.error.empty() && !evaluation.trajectory.findings.empty()) {
                const auto& finding = evaluation.trajectory.findings.front();
                result.error = finding.detail;
                if (!finding.segment_id.empty())
                    result.error += " Segment: " + finding.segment_id;
            }
            if (result.error.empty())
                result.error = "weather-coupled trajectory is not feasible";
            return result;
        }

        const auto& trajectory = evaluation.trajectory;
        if (!std::isfinite(trajectory.total_time_s)
            || trajectory.total_time_s <= 0.0
            || !std::isfinite(trajectory.total_energy_wh)
            || trajectory.total_energy_wh < 0.0
            || !std::isfinite(trajectory.remaining_energy_wh)
            || trajectory.remaining_energy_wh < 0.0) {
            result.error = "weather-corrected trajectory metrics are invalid";
            return result;
        }

        result.evaluated = true;
        result.snapshot_id = snapshot_id;
        result.snapshot_version = snapshot_version;
        result.corrected_time_s = trajectory.total_time_s;
        result.corrected_energy_wh = trajectory.total_energy_wh;
        result.remaining_energy_wh = trajectory.remaining_energy_wh;
        return result;
    };
}

} // namespace bluesky::weather
