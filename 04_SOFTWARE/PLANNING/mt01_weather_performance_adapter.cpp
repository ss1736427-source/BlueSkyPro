#include "mt01_weather_performance_adapter.hpp"

#include <algorithm>
#include <cmath>
#include <utility>

namespace bluesky::planning {
namespace {
void reject(Mt01WeatherPerformanceBindingResult& result, const std::string& code)
{
    result.diagnostics.push_back(code);
}
}

Mt01WeatherPerformanceBindingResult Mt01WeatherPerformanceAdapter::bind(
    const Mission& mission,
    const Mt01MappingPipelineResult& pipeline,
    const CoverageCameraModel& camera,
    const VehicleEquipmentCapabilityProfile& capability,
    TrajectoryPerformanceInput base)
{
    Mt01WeatherPerformanceBindingResult result;
    result.performance = std::move(base);
    if (mission.vehicle_assignments.size() != 1) {
        reject(result, "MT01_WEATHER_REQUIRES_ONE_ASSIGNED_VEHICLE"); return result;
    }
    const auto& vehicle = mission.vehicle_assignments.front();
    if (vehicle.vehicle_id.empty() || vehicle.configuration_version.empty()
        || capability.identity.vehicle_id != vehicle.vehicle_id
        || capability.configuration.configuration_version != vehicle.configuration_version
        || !capability.configuration.valid_for_mission) {
        reject(result, "MT01_WEATHER_VEHICLE_CAPABILITY_BINDING_MISMATCH"); return result;
    }
    if (result.performance.uav_id != vehicle.vehicle_id
        || result.performance.configuration_version != vehicle.configuration_version) {
        reject(result, "MT01_WEATHER_PERFORMANCE_PROFILE_BINDING_MISMATCH"); return result;
    }
    if (!pipeline.coverage_candidate_available || !pipeline.kinematic_geometry_available
        || !pipeline.route_candidate_available || !pipeline.coverage.valid
        || !pipeline.route_candidate.available) {
        reject(result, "MT01_WEATHER_COVERAGE_OR_ROUTE_CANDIDATE_UNAVAILABLE"); return result;
    }
    if (pipeline.kinematic_geometry.segments.empty()
        || pipeline.route_candidate.route.segments.size()
            != pipeline.kinematic_geometry.segments.size()) {
        reject(result, "MT01_WEATHER_ROUTE_SEGMENT_MAPPING_INCOMPLETE"); return result;
    }

    const EquipmentAssignment* assignedCamera = nullptr;
    double totalEquipmentPowerW = 0.0;
    for (const auto& assignment : mission.equipment_assignments) {
        if (assignment.vehicle_id != vehicle.vehicle_id || assignment.equipment_id.empty()
            || assignment.equipment_profile_id.empty() || assignment.configuration_version.empty()
            || assignment.compatibility_state != "COMPATIBLE"
            || assignment.verification_state != "VERIFIED") {
            reject(result, "MT01_WEATHER_EQUIPMENT_ASSIGNMENT_NOT_VERIFIED"); return result;
        }
        const auto found = std::find_if(
            capability.configuration.equipment.begin(), capability.configuration.equipment.end(),
            [&](const EquipmentCapability& item) {
                return item.equipment_id == assignment.equipment_id
                    && item.equipment_version == assignment.configuration_version;
            });
        if (found == capability.configuration.equipment.end()
            || !std::isfinite(found->electrical_power) || found->electrical_power < 0.0) {
            reject(result, "MT01_WEATHER_EQUIPMENT_CAPABILITY_OR_POWER_UNKNOWN"); return result;
        }
        totalEquipmentPowerW += found->electrical_power;
        if (!std::isfinite(totalEquipmentPowerW)) {
            reject(result, "MT01_WEATHER_EQUIPMENT_POWER_INVALID"); return result;
        }
        if (assignment.equipment_class == "CAMERA") {
            if (assignedCamera) {
                reject(result, "MT01_WEATHER_REQUIRES_EXACTLY_ONE_CAMERA"); return result;
            }
            assignedCamera = &assignment;
            if (found->sensor_camera_type.empty() || !found->triggering_capable
                || found->electrical_power <= 0.0) {
                reject(result, "MT01_WEATHER_CAMERA_CAPABILITY_OR_POWER_UNKNOWN"); return result;
            }
        }
    }
    if (!assignedCamera) {
        reject(result, "MT01_WEATHER_CAMERA_ASSIGNMENT_REQUIRED"); return result;
    }
    if (camera.camera_configuration_id != assignedCamera->equipment_id
        || camera.camera_configuration_id.empty() || camera.calibration_source_reference.empty()) {
        reject(result, "MT01_WEATHER_CAMERA_IDENTITY_OR_PROVENANCE_MISMATCH"); return result;
    }
    if (!std::isfinite(camera.target_gsd_cm_px) || camera.target_gsd_cm_px <= 0.0
        || !std::isfinite(camera.minimum_trigger_interval_s) || camera.minimum_trigger_interval_s <= 0.0
        || !std::isfinite(camera.exposure_time_s) || camera.exposure_time_s <= 0.0
        || !std::isfinite(camera.maximum_motion_blur_px) || camera.maximum_motion_blur_px <= 0.0
        || !std::isfinite(pipeline.coverage.metrics.forward_trigger_spacing_m)
        || pipeline.coverage.metrics.forward_trigger_spacing_m <= 0.0) {
        reject(result, "MT01_WEATHER_CAMERA_CAPTURE_PARAMETERS_INCOMPLETE"); return result;
    }

    const auto& route = pipeline.route_candidate.route;
    std::vector<std::string> captureSegments;
    for (std::size_t i = 0; i < pipeline.kinematic_geometry.segments.size(); ++i) {
        const auto& geometrySegment = pipeline.kinematic_geometry.segments[i];
        const auto& routeSegment = route.segments[i];
        if (geometrySegment.segment_id.empty() || geometrySegment.segment_id != routeSegment.segment_id) {
            reject(result, "MT01_WEATHER_ROUTE_SEGMENT_IDENTITY_MISMATCH"); return result;
        }
        if (geometrySegment.type == Mt01KinematicSegmentType::Coverage)
            captureSegments.push_back(routeSegment.segment_id);
    }
    if (captureSegments.empty()) {
        reject(result, "MT01_WEATHER_NO_CAMERA_COVERAGE_SEGMENTS"); return result;
    }

    result.performance.payload_power_w = totalEquipmentPowerW;
    result.performance.camera_configuration_id = camera.camera_configuration_id;
    result.performance.camera_gsd_m_px = camera.target_gsd_cm_px / 100.0;
    result.performance.camera_forward_trigger_spacing_m = pipeline.coverage.metrics.forward_trigger_spacing_m;
    result.performance.camera_minimum_trigger_interval_s = camera.minimum_trigger_interval_s;
    result.performance.camera_exposure_time_s = camera.exposure_time_s;
    result.performance.camera_maximum_motion_blur_px = camera.maximum_motion_blur_px;
    result.performance.camera_capture_segment_ids = std::move(captureSegments);
    result.valid = true;
    result.diagnostics.push_back(
        "MT01_WEATHER_CAMERA_AND_EQUIPMENT_BOUND: camera limits and assigned equipment power are bound to exact vehicle/equipment revisions.");
    return result;
}

} // namespace bluesky::planning
