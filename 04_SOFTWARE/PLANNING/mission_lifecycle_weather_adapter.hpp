#pragma once

#include "mission_lifecycle.hpp"
#include "../WEATHER/weather_trajectory_calculator.hpp"

#include <algorithm>
#include <cmath>
#include <string>

namespace bluesky::planning {
namespace lifecycle_detail {

inline bool weatherTrajectoryMatches(
    const Route &route,
    const weather::WeatherTrajectoryEvaluation &evaluation,
    const TrajectoryPerformanceInput &input,
    const std::string &vehicleConfigurationId)
{
    const auto &trajectory = evaluation.trajectory;
    const double energyTolerance = std::max(1e-6, input.usable_energy_wh * 1e-9);
    const bool completeRoute = route.trajectory_geometry_complete
        && route.waypoints.size() >= 2
        && route.segments.size() + 1 == route.waypoints.size();
    const bool validMetrics = std::isfinite(trajectory.total_time_s)
        && trajectory.total_time_s > 0.0
        && std::isfinite(trajectory.total_energy_wh)
        && trajectory.total_energy_wh >= 0.0
        && std::isfinite(trajectory.remaining_energy_wh)
        && trajectory.remaining_energy_wh >= 0.0
        && std::isfinite(input.usable_energy_wh)
        && input.usable_energy_wh > 0.0
        && std::isfinite(input.reserve_requirement_wh)
        && input.reserve_requirement_wh >= 0.0
        && std::abs((trajectory.total_energy_wh + trajectory.remaining_energy_wh)
                    - input.usable_energy_wh) <= energyTolerance;
    return evaluation.weather_ready
        && trajectory.status == TrajectoryStatus::Feasible
        && completeRoute
        && !route.lineage.mission_id.empty()
        && !route.lineage.mission_version.empty()
        && !route.lineage.route_id.empty()
        && !route.lineage.route_version.empty()
        && !route.lineage.calculation_input_version.empty()
        && trajectory.route_id == route.lineage.route_id
        && trajectory.route_version == route.lineage.route_version
        && trajectory.mission_start_epoch_s > 0
        && !input.uav_id.empty()
        && !input.configuration_version.empty()
        && !input.performance_version.empty()
        && trajectory.uav_id == input.uav_id
        && trajectory.configuration_version == input.configuration_version
        && trajectory.performance_version == input.performance_version
        && !vehicleConfigurationId.empty()
        && trajectory.wind_snapshot_id == input.wind_snapshot_id
        && !input.wind_snapshot_id.empty()
        && trajectory.wind_snapshot_version == input.wind_snapshot_version
        && !input.wind_snapshot_version.empty()
        && !trajectory.calculation_version.empty()
        && !trajectory.dependency_identity.empty()
        && trajectory.usable_energy_wh == input.usable_energy_wh
        && trajectory.reserve_requirement_wh == input.reserve_requirement_wh
        && trajectory.segments.size() > 0
        && trajectory.points.size() == trajectory.segments.size() + 1
        && validMetrics;
}

inline MissionLifecycleValidationEvidence makeWeatherAttestation(
    const Route &route,
    const weather::WeatherTrajectoryEvaluation &evaluation,
    const TrajectoryPerformanceInput &input,
    const std::string &vehicleConfigurationId,
    const std::string &validatorId)
{
    MissionLifecycleValidationEvidence evidence;
    const auto &trajectory = evaluation.trajectory;
    evidence.result_id = validatorId + ":" + trajectory.calculation_version
        + ":" + trajectory.route_id + "@" + trajectory.route_version
        + ":" + trajectory.wind_snapshot_id + "@" + trajectory.wind_snapshot_version
        + ":" + trajectory.dependency_identity;
    evidence.mission_id = route.lineage.mission_id;
    evidence.mission_version = route.lineage.mission_version;
    evidence.route_id = route.lineage.route_id;
    evidence.route_version = route.lineage.route_version;
    evidence.calculation_input_version = route.lineage.calculation_input_version;
    evidence.validator_id = validatorId;
    evidence.validator_version = "1";
    evidence.snapshot_id = trajectory.wind_snapshot_id;
    evidence.source_reference = "forecast:" + trajectory.wind_snapshot_id
        + "@" + trajectory.wind_snapshot_version
        + ";performance:" + trajectory.performance_version
        + ";calculation:" + trajectory.calculation_version
        + ";dependency:" + trajectory.dependency_identity;
    evidence.vehicle_configuration_id = vehicleConfigurationId;
    evidence.status = weatherTrajectoryMatches(
        route, evaluation, input, vehicleConfigurationId)
        ? MissionLifecycleValidationStatus::Passed
        : MissionLifecycleValidationStatus::Failed;
    return evidence;
}

} // namespace lifecycle_detail

inline MissionLifecycleValidationEvidence makeWeatherValidationEvidence(
    const Route &route,
    const weather::WeatherTrajectoryEvaluation &evaluation,
    const TrajectoryPerformanceInput &input,
    const std::string &vehicleConfigurationId)
{
    return lifecycle_detail::makeWeatherAttestation(
        route, evaluation, input, vehicleConfigurationId,
        "weather-trajectory-calculator");
}

inline MissionLifecycleValidationEvidence makeEnergyReserveValidationEvidence(
    const Route &route,
    const weather::WeatherTrajectoryEvaluation &evaluation,
    const TrajectoryPerformanceInput &input,
    const std::string &vehicleConfigurationId)
{
    auto evidence = lifecycle_detail::makeWeatherAttestation(
        route, evaluation, input, vehicleConfigurationId,
        "wind-performance-energy-reserve");
    if (evidence.status == MissionLifecycleValidationStatus::Passed
        && evaluation.trajectory.remaining_energy_wh < input.reserve_requirement_wh) {
        evidence.status = MissionLifecycleValidationStatus::Failed;
    }
    return evidence;
}

} // namespace bluesky::planning
