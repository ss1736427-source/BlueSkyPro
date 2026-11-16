#pragma once
#include "mission_finalization_runtime.hpp"
#include "mission_lifecycle_mt01_quality_adapter.hpp"
#include "mission_lifecycle_regulatory_adapter.hpp"
#include "mission_lifecycle_route_adapter.hpp"
#include "mission_lifecycle_trajectory_geometry_adapter.hpp"
#include "mission_lifecycle_notam_adapter.hpp"
#include "mission_lifecycle_terrain_adapter.hpp"
#include "mission_lifecycle_vehicle_adapter.hpp"
#include "mission_lifecycle_weather_adapter.hpp"
#include "notam_prohibited_zone_validator.hpp"
#include "orchestrator/algorithm_orchestrator.hpp"
#include "../WEATHER/weather_trajectory_calculator.hpp"
#include "route_constraint_validator.hpp"
#include <algorithm>
#include <optional>
#include <string>
#include <utility>
namespace bluesky::planning {
struct MissionLifecycleMt01QualityInputs {
    CoverageMissionInput mission;
    CoveragePlanResult coverage;
    Mt01KinematicGeometryResult kinematic_geometry;
};
struct MissionLifecycleCoordinatorResult {
    MissionFinalizationRuntimeResult finalization;
    bool mt01_quality_evaluated{false};
    std::vector<std::string> evidence_assembly_blockers;
};

// Explicit references to authoritative outputs. Null means that a service did
// not produce the required result; the coordinator never manufactures PASS.
struct MissionLifecycleAuthoritativeSources {
    const OrchestratorDecision* planning_decision{nullptr};
    const NotamSnapshot* notam_snapshot{nullptr};
    const TerrainDemSnapshot* terrain_snapshot{nullptr};
    const TerrainProfileValidationResult* terrain_validation{nullptr};
    const weather::WeatherTrajectoryEvaluation* weather_evaluation{nullptr};
    const TrajectoryPerformanceInput* performance_input{nullptr};
    std::string vehicle_configuration_id;
};
// Single trusted C++ entry point for constructing the final validation bundle.
// NOTAM, terrain, vehicle, weather/energy and non-MT01 task-quality evidence
// must still be supplied by their own authoritative services in 'evidence'.
class MissionLifecycleValidationCoordinator {
public:
    // Production-facing path: assemble lifecycle attestations from the exact
    // selected orchestrator candidate and immutable NOTAM/DEM/weather results.
    // Missing or mismatched service outputs fail before the READY transition.
    static MissionLifecycleCoordinatorResult finalizeReadyFromAuthoritativeSources(
        MissionLifecycleRecord& record,
        MissionLifecycleEvidence evidence,
        const MissionLifecycleAuthoritativeSources& sources,
        const RegulatoryComplianceAssessment& regulatory_assessment,
        std::uint64_t now_epoch_s,
        const FlightProfile& selected_flight_profile,
        const VehicleCompilationInput& assigned_vehicle,
        const std::string& actor_id,
        const std::string& timestamp,
        const std::string& reason,
        const std::optional<MissionLifecycleMt01QualityInputs>& mt01_quality = std::nullopt)
    {
        MissionLifecycleCoordinatorResult result;
        const auto block = [&](const char* code) {
            result.evidence_assembly_blockers.emplace_back(code);
        };
        const auto* decision = sources.planning_decision;
        if (!decision || decision->feasibility != Feasibility::Feasible
            || !decision->selected_route || !decision->selected_flight_safety_validation
            || !decision->selected_notam_validation
            || !decision->selected_weather_trajectory
            || !decision->weather_assessment_applied)
            block("AUTHORITATIVE_PLANNING_DECISION_INCOMPLETE");
        if (!sources.notam_snapshot)
            block("NOTAM_SNAPSHOT_REQUIRED");
        if (!sources.terrain_snapshot || !sources.terrain_validation)
            block("TERRAIN_VALIDATION_SOURCE_REQUIRED");
        if (!sources.weather_evaluation || !sources.performance_input)
            block("WEATHER_ENERGY_VALIDATION_SOURCE_REQUIRED");
        if (sources.vehicle_configuration_id.empty())
            block("VEHICLE_CONFIGURATION_ID_REQUIRED");
        if (!result.evidence_assembly_blockers.empty()) {
            result.finalization.transition.state = record.state;
            result.finalization.transition.blockers = result.evidence_assembly_blockers;
            return result;
        }

        const Route& route = *decision->selected_route;
        const auto& weatherEvaluation = *sources.weather_evaluation;
        const auto& weatherTrajectory = weatherEvaluation.trajectory;
        const auto& selectedTrajectory = *decision->selected_weather_trajectory;
        const bool selectedWeatherMatches =
            selectedTrajectory.route_id == route.lineage.route_id
            && selectedTrajectory.route_version == route.lineage.route_version
            && selectedTrajectory.wind_snapshot_id == weatherTrajectory.wind_snapshot_id
            && selectedTrajectory.wind_snapshot_version == weatherTrajectory.wind_snapshot_version
            && weatherTrajectory.route_id == route.lineage.route_id
            && weatherTrajectory.route_version == route.lineage.route_version
            && decision->weather_snapshot_id == weatherTrajectory.wind_snapshot_id
            && decision->weather_snapshot_version == weatherTrajectory.wind_snapshot_version
            && sources.performance_input->configuration_version == sources.vehicle_configuration_id;
        if (!selectedWeatherMatches)
            block("WEATHER_RESULT_NOT_BOUND_TO_SELECTED_ROUTE_OR_CONFIGURATION");
        if (route.environment.airspace_snapshot_id != sources.notam_snapshot->snapshot_id)
            block("NOTAM_SNAPSHOT_NOT_BOUND_TO_SELECTED_ROUTE");
        if (!result.evidence_assembly_blockers.empty()) {
            result.finalization.transition.state = record.state;
            result.finalization.transition.blockers = result.evidence_assembly_blockers;
            return result;
        }

        evidence.notam_validation = makeNotamValidationEvidence(
            route, *sources.notam_snapshot, *decision->selected_notam_validation);
        evidence.notam_recheck_snapshot_id = sources.notam_snapshot->snapshot_id;
        evidence.mission_notam_snapshot_id = sources.notam_snapshot->snapshot_id;
        evidence.mission_notam_snapshot_current =
            evidence.notam_validation.status == MissionLifecycleValidationStatus::Passed;
        evidence.notam_recheck_complete = evidence.mission_notam_snapshot_current;

        evidence.terrain_validation = makeTerrainValidationEvidence(
            route, *sources.terrain_snapshot, *sources.terrain_validation);
        evidence.terrain_snapshot_id = sources.terrain_snapshot->snapshot_id;
        evidence.terrain_snapshot_current =
            evidence.terrain_validation.status == MissionLifecycleValidationStatus::Passed;

        evidence.vehicle_envelope_validation = makeVehicleEnvelopeValidationEvidence(
            route, *decision->selected_flight_safety_validation);
        evidence.vehicle_configuration_id = sources.vehicle_configuration_id;
        evidence.weather_validation = makeWeatherValidationEvidence(
            route, weatherEvaluation, *sources.performance_input,
            sources.vehicle_configuration_id);
        evidence.energy_reserve_validation = makeEnergyReserveValidationEvidence(
            route, weatherEvaluation, *sources.performance_input,
            sources.vehicle_configuration_id);
        evidence.weather_snapshot_id = weatherTrajectory.wind_snapshot_id;
        evidence.weather_assessment_complete =
            evidence.weather_validation.status == MissionLifecycleValidationStatus::Passed;
        evidence.weather_aware_route_generated = evidence.weather_assessment_complete;
        evidence.route_candidate_selected = true;
        evidence.baseline_route_generated = true;

        return finalizeReady(record, std::move(evidence), route,
            regulatory_assessment, now_epoch_s, selected_flight_profile,
            assigned_vehicle, actor_id, timestamp, reason, mt01_quality);
    }

    static MissionLifecycleCoordinatorResult finalizeReady(
        MissionLifecycleRecord& record,
        MissionLifecycleEvidence evidence,
        const Route& selected_route,
        const RegulatoryComplianceAssessment& regulatory_assessment,
        std::uint64_t now_epoch_s,
        const FlightProfile& selected_flight_profile,
        const VehicleCompilationInput& assigned_vehicle,
        const std::string& actor_id,
        const std::string& timestamp,
        const std::string& reason,
        const std::optional<MissionLifecycleMt01QualityInputs>& mt01_quality = std::nullopt)
    {
        MissionLifecycleCoordinatorResult result;
        const auto structural = RouteConstraintValidator::validate(selected_route);
        evidence.route_structure_validation =
            makeRouteStructureValidationEvidence(selected_route, structural);
        evidence.trajectory_geometry_validation =
            makeTrajectoryGeometryValidationEvidence(selected_route);
        evidence.regulatory_validation =
            makeRegulatoryValidationEvidence(regulatory_assessment, now_epoch_s);

        const bool isMt01 = std::find(record.mission_template_ids.begin(),
            record.mission_template_ids.end(), "MT-01") != record.mission_template_ids.end();
        if (isMt01) {
            result.mt01_quality_evaluated = mt01_quality.has_value();
            evidence.task_quality_validation = mt01_quality
                ? makeMt01TaskQualityValidationEvidence(selected_route,
                    mt01_quality->mission, mt01_quality->coverage,
                    mt01_quality->kinematic_geometry)
                : MissionLifecycleValidationEvidence{};
        }

        result.finalization = MissionFinalizationRuntime::finalizeReady(
            record, std::move(evidence), selected_flight_profile, assigned_vehicle,
            actor_id, timestamp, reason);
        return result;
    }
};
} // namespace bluesky::planning
