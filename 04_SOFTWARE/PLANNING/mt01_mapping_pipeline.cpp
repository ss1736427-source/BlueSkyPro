#include "mt01_mapping_pipeline.hpp"
#include "mt01_kinematic_geometry_builder.hpp"
#include "notam_geometry_adapter.hpp"

#include <cmath>

#include <algorithm>
#include <utility>

namespace bluesky::planning {
namespace {

void addFinding(Mt01MappingPipelineResult& result, const std::string& finding)
{
    result.findings.push_back(finding);
}

} // namespace

Mt01MappingPipelineResult Mt01MappingPipeline::calculate(
    const Mt01MappingRequest& request)
{
    Mt01MappingPipelineResult result;
    const auto& mission = request.mission;

    if (mission.mission_mode != "AUTO" && mission.mission_mode != "MANUAL")
        addFinding(result, "MISSION_MODE_INVALID");
    if (mission.identity.mission_id.empty() || mission.identity.mission_version.empty())
        addFinding(result, "MISSION_IDENTITY_REQUIRED");
    if (mission.task.description.empty())
        addFinding(result, "TASK_DESCRIPTION_REQUIRED");
    if (mission.intent.objective.empty() || mission.intent.task_type.empty())
        addFinding(result, "MISSION_INTENT_REQUIRED");

    if (std::find(mission.selected_template_ids.begin(),
                  mission.selected_template_ids.end(), "MT-01")
        == mission.selected_template_ids.end()) {
        addFinding(result, "MT01_TEMPLATE_NOT_SELECTED");
    }

    if (mission.vehicle_assignments.size() != 1) {
        addFinding(result, "MT01_REQUIRES_EXACTLY_ONE_ASSIGNED_VEHICLE");
    } else {
        const auto& vehicle = mission.vehicle_assignments.front();
        if (vehicle.vehicle_id.empty() || vehicle.configuration_id.empty()
            || vehicle.configuration_version.empty()) {
            addFinding(result, "VEHICLE_CONFIGURATION_BINDING_REQUIRED");
        }
        if (vehicle.configuration_type != "STANDARD"
            && vehicle.configuration_type != "CUSTOM"
            && vehicle.configuration_type != "DERIVED") {
            addFinding(result, "VEHICLE_CONFIGURATION_TYPE_INVALID");
        }
        if (vehicle.validation_state != "VALIDATED"
            && vehicle.validation_state != "APPROVED") {
            addFinding(result, "VEHICLE_CONFIGURATION_NOT_VALIDATED");
        }
        if (vehicle.configuration_type == "STANDARD"
            && vehicle.approval_state != "APPROVED") {
            addFinding(result, "STANDARD_CONFIGURATION_NOT_APPROVED");
        }
        if (vehicle.configuration_type != "STANDARD"
            && vehicle.approval_state != "APPROVED"
            && vehicle.approval_state != "PENDING") {
            addFinding(result, "CUSTOM_CONFIGURATION_APPROVAL_STATE_INVALID");
        }
    }

    const EquipmentAssignment* camera = nullptr;
    for (const auto& equipment : mission.equipment_assignments) {
        if (equipment.equipment_class != "CAMERA")
            continue;
        if (camera) {
            addFinding(result, "MT01_REQUIRES_EXACTLY_ONE_ASSIGNED_CAMERA");
            break;
        }
        camera = &equipment;
    }

    if (!camera) {
        addFinding(result, "MT01_CAMERA_EQUIPMENT_ASSIGNMENT_REQUIRED");
    } else {
        if (camera->vehicle_id.empty() || camera->equipment_id.empty()
            || camera->equipment_profile_id.empty()
            || camera->configuration_version.empty()) {
            addFinding(result, "CAMERA_EQUIPMENT_BINDING_REQUIRED");
        }
        if (mission.vehicle_assignments.size() == 1
            && camera->vehicle_id != mission.vehicle_assignments.front().vehicle_id) {
            addFinding(result, "CAMERA_ASSIGNED_TO_DIFFERENT_VEHICLE");
        }
        if (camera->compatibility_state != "COMPATIBLE")
            addFinding(result, "CAMERA_COMPATIBILITY_NOT_CONFIRMED");
        if (camera->verification_state != "VERIFIED")
            addFinding(result, "CAMERA_EQUIPMENT_NOT_VERIFIED");
    }

    if (request.camera.camera_configuration_id.empty()
        || request.camera.calibration_source_reference.empty()) {
        addFinding(result, "CAMERA_CONFIGURATION_AND_SOURCE_PROVENANCE_REQUIRED");
    }
    if (camera && !request.camera.camera_configuration_id.empty()
        && request.camera.camera_configuration_id != camera->equipment_id) {
        addFinding(result, "CAMERA_ASSIGNMENT_MISMATCH");
    }

    const MissionGeometryObject* aoi = nullptr;
    for (const auto& geometry : mission.geometry_objects) {
        if (geometry.geometry_type != "POLYGON_AOI")
            continue;
        if (aoi) {
            addFinding(result, "MT01_REQUIRES_EXACTLY_ONE_POLYGON_AOI");
            break;
        }
        aoi = &geometry;
    }

    std::vector<GeoPoint> area;
    if (!aoi) {
        addFinding(result, "MT01_POLYGON_AOI_REQUIRED");
    } else {
        if (aoi->geometry_id.empty() || aoi->coordinate_reference_system != "EPSG:4326")
            addFinding(result, "MT01_AOI_ID_OR_CRS_INVALID");
        if (std::find(aoi->template_ids.begin(), aoi->template_ids.end(), "MT-01")
            == aoi->template_ids.end())
            addFinding(result, "MT01_AOI_TEMPLATE_ASSOCIATION_REQUIRED");
        area.reserve(aoi->coordinates.size());
        for (const auto& coordinate : aoi->coordinates)
            area.push_back({coordinate.latitude_deg, coordinate.longitude_deg});
    }

    if (!result.findings.empty())
        return result;

    // Derive the same camera-based AGL target used by CoveragePathPlanner so
    // only vertically applicable NOTAM zones are removed from the coverage domain.
    const double targetGsdM = request.camera.target_gsd_cm_px / 100.0;
    const double targetAltitudeAglM =
        targetGsdM * (request.camera.focal_length_mm * 1e-3) /
        (request.camera.pixel_pitch_um * 1e-6);
    const NotamGeometryAdapterInput notamInput{
        &request.notam_snapshot,
        request.planned_mission_start_epoch_s,
        request.planned_mission_end_epoch_s,
        targetAltitudeAglM,
        request.notam_safety_buffer_m,
        area
    };
    result.notam_geometry = NotamGeometryAdapter::build(notamInput);
    if (!result.notam_geometry.valid) {
        for (const auto& diagnostic : result.notam_geometry.diagnostics)
            addFinding(result, diagnostic);
        return result;
    }

    auto combinedExclusions = request.exclusion_areas;
    combinedExclusions.insert(combinedExclusions.end(),
        result.notam_geometry.exclusion_areas.begin(),
        result.notam_geometry.exclusion_areas.end());
    auto routeMetadata = request.route_candidate_metadata;
    if (!routeMetadata.environment.airspace_snapshot_id.empty() &&
        routeMetadata.environment.airspace_snapshot_id != result.notam_geometry.snapshot_id) {
        addFinding(result, "MT01_NOTAM_SNAPSHOT_LINEAGE_MISMATCH");
        return result;
    }
    routeMetadata.environment.airspace_snapshot_id = result.notam_geometry.snapshot_id;

    CoverageMissionInput coverageInput;
    coverageInput.mission_id = mission.identity.mission_id;
    coverageInput.mission_version = mission.identity.mission_version;
    coverageInput.area_of_interest = std::move(area);
    coverageInput.exclusion_areas = combinedExclusions;
    coverageInput.camera = request.camera;
    coverageInput.minimum_altitude_agl_m = request.minimum_altitude_agl_m;
    coverageInput.maximum_altitude_agl_m = request.maximum_altitude_agl_m;
    coverageInput.cruise_speed_mps = request.cruise_speed_mps;
    coverageInput.line_angle_deg = request.line_angle_deg;

    result.coverage = CoveragePathPlanner::generate(coverageInput);
    if (!result.coverage.valid) {
        for (const auto& diagnostic : result.coverage.diagnostics)
            addFinding(result, diagnostic);
        return result;
    }

    result.status = Mt01PipelineStatus::CoverageCandidateAvailable;
    result.coverage_candidate_available = true;
    result.flight_release_eligible = false;
    if (mission.vehicle_assignments.front().configuration_type != "STANDARD"
        && mission.vehicle_assignments.front().approval_state == "PENDING") {
        addFinding(result,
            "CUSTOM_CONFIGURATION_APPROVAL_PENDING: analysis candidate only; operational use remains blocked until the required approval is completed.");
    }
    const auto& assignedVehicle = mission.vehicle_assignments.front();
    double connectorSpeedMps = request.vehicle_motion_model.reference_speed_mps;
    double connectorMinimumSpeedMps = request.vehicle_motion_model.minimum_speed_mps;
    if (!std::isfinite(request.requested_max_turn_radius_m)
        || request.requested_max_turn_radius_m < 0.0) {
        addFinding(result, "MT01_TURN_RADIUS_CONSTRAINT_INVALID");
        return result;
    }
    if (request.requested_max_turn_radius_m > 0.0) {
        const auto turnSpeed = VehicleMotionModelEvaluator::recommendTurnSpeedForRadius(
            request.vehicle_motion_model,
            request.requested_max_turn_radius_m,
            request.fixed_wing_clean_stall_speed_mps,
            request.fixed_wing_stall_speed_reference,
            request.fixed_wing_stall_margin_factor);
        if (!turnSpeed.valid || !turnSpeed.may_satisfy_motion_hard_gate) {
            addFinding(result, "MT01_TURN_SPEED_CAP_UNAVAILABLE: "
                + turnSpeed.diagnostic);
            return result;
        }
        connectorSpeedMps = turnSpeed.selected_speed_mps;
        connectorMinimumSpeedMps = turnSpeed.minimum_safe_turn_speed_mps;
        result.turn_speed_constraint_applied = true;
    }
    result.selected_connector_speed_mps = connectorSpeedMps;
    result.minimum_safe_connector_speed_mps = connectorMinimumSpeedMps;
    result.kinematic_geometry = Mt01KinematicGeometryBuilder::build(
        result.coverage,
        request.vehicle_motion_model,
        assignedVehicle.vehicle_id,
        assignedVehicle.configuration_id,
        request.cruise_speed_mps,
        combinedExclusions,
        2.0,
        connectorSpeedMps,
        connectorMinimumSpeedMps);
    result.kinematic_geometry_available = result.kinematic_geometry.feasible;
    for (const auto& diagnostic : result.kinematic_geometry.diagnostics)
        addFinding(result, diagnostic);
    if (!result.kinematic_geometry_available) {
        addFinding(result,
            "MT01_KINEMATIC_GEOMETRY_BLOCKED: coverage legs remain available for analysis, but a validated fixed-wing turn-connector candidate was not established.");
    } else {
        const bool connectorSlowerThanCoverage =
            connectorSpeedMps + 1e-6 < request.cruise_speed_mps;
        const SpeedProfileResult* speedProfile = nullptr;
        if (connectorSlowerThanCoverage) {
            if (!request.longitudinal_dynamics_validated
                || request.longitudinal_dynamics_source.empty()
                || !std::isfinite(request.maximum_acceleration_mps2)
                || request.maximum_acceleration_mps2 <= 0.0
                || !std::isfinite(request.maximum_deceleration_mps2)
                || request.maximum_deceleration_mps2 <= 0.0
                || !std::isfinite(request.initial_route_speed_mps)
                || request.initial_route_speed_mps <= 0.0
                || !std::isfinite(request.final_route_speed_mps)
                || request.final_route_speed_mps <= 0.0) {
                addFinding(result,
                    "MT01_SPEED_PROFILE_REQUIRED: a slower turn requires validated longitudinal acceleration/deceleration evidence and explicit route boundary speeds before a canonical route can be published.");
            } else {
                SpeedProfileInput profileInput;
                profileInput.vehicle_model_id = request.vehicle_motion_model.model_id;
                profileInput.vehicle_model_version = request.vehicle_motion_model.model_version;
                profileInput.configuration_id = request.vehicle_motion_model.configuration_id;
                profileInput.speed_reference = request.vehicle_motion_model.speed_reference;
                profileInput.longitudinal_dynamics_validated = true;
                profileInput.longitudinal_dynamics_source = request.longitudinal_dynamics_source;
                profileInput.maximum_acceleration_mps2 = request.maximum_acceleration_mps2;
                profileInput.maximum_deceleration_mps2 = request.maximum_deceleration_mps2;
                profileInput.initial_speed_mps = request.initial_route_speed_mps;
                profileInput.final_speed_mps = request.final_route_speed_mps;
                profileInput.segments.reserve(result.kinematic_geometry.segments.size());
                for (const auto& segment : result.kinematic_geometry.segments) {
                    profileInput.segments.push_back({
                        segment.segment_id,
                        segment.distance_m,
                        segment.minimum_speed_mps,
                        segment.nominal_speed_mps
                    });
                }
                result.speed_profile = VariableSpeedProfileSolver::calculate(profileInput);
                if (result.speed_profile.status == SpeedProfileStatus::Feasible) {
                    result.speed_profile_available = true;
                    speedProfile = &result.speed_profile;
                } else {
                    for (const auto& finding : result.speed_profile.findings)
                        addFinding(result, finding);
                    addFinding(result,
                        "MT01_SPEED_PROFILE_INFEASIBLE: the selected turn speed cannot be reached within validated longitudinal dynamics and available segment distances.");
                }
            }
        }
        if (!connectorSlowerThanCoverage || speedProfile) {
            result.route_candidate = Mt01RouteCandidateBuilder::build(
                result.kinematic_geometry,
                result.coverage,
                mission.identity.mission_id,
                mission.identity.mission_version,
                routeMetadata,
                speedProfile);
            result.route_candidate_available = result.route_candidate.available;
            for (const auto& diagnostic : result.route_candidate.diagnostics)
                addFinding(result, diagnostic);
            if (!result.route_candidate_available) {
                addFinding(result,
                    "MT01_CANONICAL_ROUTE_CANDIDATE_BLOCKED: complete route/generator/input lineage and canonical structure are required.");
            }
        }
        if (result.route_candidate_available && !request.task_execution_events.empty()) {
            // A camera exposure is not a waypoint dwell or task execution stop.
            // Do not inject per-image duration/energy into RouteWaypoint costs.
            result.task_execution_events_applied = false;
            addFinding(result,
                "MT01_CAMERA_EXPOSURE_DWELL_MODEL_REJECTED: camera captures occur in motion; use camera trigger spacing, shutter/blur constraints and continuous payload power instead of waypoint task duration.");
        }

        const auto& envelope = request.flight_safety_envelope;
        const bool exactSafetyBinding =
            envelope.vehicle_model_id == assignedVehicle.vehicle_id
            && envelope.vehicle_model_id == request.vehicle_motion_model.model_id
            && envelope.model_version == request.vehicle_motion_model.model_version
            && envelope.configuration_id == assignedVehicle.configuration_id
            && envelope.configuration_id == request.vehicle_motion_model.configuration_id
            && envelope.motion_class == request.vehicle_motion_model.motion_class
            && envelope.speed_reference == request.vehicle_motion_model.speed_reference;
        if (!exactSafetyBinding) {
            result.flight_safety_validation.status = FlightSafetyStatus::Unknown;
            result.flight_safety_validation.vehicle_model_id = envelope.vehicle_model_id;
            result.flight_safety_validation.model_version = envelope.model_version;
            result.flight_safety_validation.configuration_id = envelope.configuration_id;
            result.flight_safety_validation.source_reference = envelope.source_reference;
            result.flight_safety_validation.findings.push_back({
                FlightSafetyParameter::ModelEvidence, {}, {}, 0.0, 0.0,
                "Safety envelope identity/configuration does not exactly match the assigned vehicle and kinematic model."
            });
            addFinding(result,
                "MT01_SEGMENT_DYNAMICS_UNKNOWN: validated safety envelope is missing or does not match the assigned vehicle configuration.");
        } else {
            std::vector<FlightSafetySegmentState> segmentStates;
            segmentStates.reserve(result.kinematic_geometry.segments.size());
            for (const auto& segment : result.kinematic_geometry.segments) {
                segmentStates.push_back({
                    segment.segment_id,
                    std::optional<double>{segment.nominal_speed_mps},
                    std::optional<double>{segment.bank_angle_rad},
                    std::optional<double>{segment.lateral_acceleration_mps2}
                });
            }
            result.flight_safety_validation =
                FlightSafetyConstraintValidator::validate(envelope, segmentStates);
            result.segment_dynamics_validated =
                result.flight_safety_validation.status == FlightSafetyStatus::Allowed;
            if (!result.segment_dynamics_validated) {
                addFinding(result,
                    result.flight_safety_validation.status == FlightSafetyStatus::Rejected
                        ? "MT01_SEGMENT_DYNAMICS_REJECTED: one or more trajectory segments exceed the validated aircraft envelope."
                        : "MT01_SEGMENT_DYNAMICS_UNKNOWN: validated speed/bank/acceleration evidence is incomplete.");
                for (const auto& finding : result.flight_safety_validation.findings)
                    addFinding(result, finding.detail);
            } else if (result.flight_safety_validation.maximum_regime_warning_required) {
                addFinding(result, result.flight_safety_validation.operator_warning);
            }
        }
    }
    addFinding(result,
        "FLIGHT_RELEASE_BLOCKED: this geometry stage does not perform wind-ground-track correction, DEM terrain following, obstacle clearance, current airspace/NOTAM validation, energy/reserve assessment or independent mission-package checks.");
    if (!request.camera.intrinsic_calibration_verified)
        addFinding(result,
            "CAMERA_CALIBRATION_UNVERIFIED: manufacturer optical geometry is not an individual camera/lens calibration.");

    return result;
}

} // namespace bluesky::planning
