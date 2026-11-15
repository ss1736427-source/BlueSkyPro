#pragma once
#include "mission_finalization_runtime.hpp"
#include "mission_lifecycle_mt01_quality_adapter.hpp"
#include "mission_lifecycle_regulatory_adapter.hpp"
#include "mission_lifecycle_route_adapter.hpp"
#include "mission_lifecycle_trajectory_geometry_adapter.hpp"
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
};
// Single trusted C++ entry point for constructing the final validation bundle.
// NOTAM, terrain, vehicle, weather/energy and non-MT01 task-quality evidence
// must still be supplied by their own authoritative services in 'evidence'.
class MissionLifecycleValidationCoordinator {
public:
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
