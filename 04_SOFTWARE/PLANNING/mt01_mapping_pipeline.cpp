#include "mt01_mapping_pipeline.hpp"
#include "mt01_kinematic_geometry_builder.hpp"

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

    CoverageMissionInput coverageInput;
    coverageInput.mission_id = mission.identity.mission_id;
    coverageInput.mission_version = mission.identity.mission_version;
    coverageInput.area_of_interest = std::move(area);
    coverageInput.exclusion_areas = request.exclusion_areas;
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
    result.kinematic_geometry = Mt01KinematicGeometryBuilder::build(
        result.coverage,
        request.vehicle_motion_model,
        assignedVehicle.vehicle_id,
        assignedVehicle.configuration_id,
        request.cruise_speed_mps,
        request.exclusion_areas);
    result.kinematic_geometry_available = result.kinematic_geometry.feasible;
    for (const auto& diagnostic : result.kinematic_geometry.diagnostics)
        addFinding(result, diagnostic);
    if (!result.kinematic_geometry_available) {
        addFinding(result,
            "MT01_KINEMATIC_GEOMETRY_BLOCKED: coverage legs remain available for analysis, but a validated fixed-wing turn-connector candidate was not established.");
    }
    if (!request.camera.intrinsic_calibration_verified)
        addFinding(result,
            "CAMERA_CALIBRATION_UNVERIFIED: manufacturer optical geometry is not an individual camera/lens calibration.");

    return result;
}

} // namespace bluesky::planning
