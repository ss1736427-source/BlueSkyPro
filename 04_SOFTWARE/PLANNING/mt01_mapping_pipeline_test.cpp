#include "mt01_mapping_pipeline.hpp"

#include <cassert>
#include <cmath>

using namespace bluesky::planning;

static Mt01MappingRequest makeRequest()
{
    Mt01MappingRequest request;
    auto& mission = request.mission;
    mission.identity.mission_id = "MT01-MAVIC3E-TEST";
    mission.identity.mission_version = "1";
    mission.mission_mode = "AUTO";
    mission.selected_template_ids = {"MT-01"};
    mission.intent.objective = "Photogrammetric orthomosaic";
    mission.intent.task_type = "MAPPING";
    mission.task.description = "Map the defined area at 3 cm GSD";
    mission.vehicle_assignments.push_back({"DJI-MAVIC-3E", "MAVIC-3E-STANDARD-REFERENCE-001"});
    mission.payload_assignments.push_back({"DJI-MAVIC-3E-WIDE", "MAVIC-3E-WIDE-REFERENCE-001"});

    MissionGeometryObject aoi;
    aoi.geometry_id = "AOI-01";
    aoi.geometry_type = "POLYGON_AOI";
    aoi.coordinate_reference_system = "EPSG:4326";
    aoi.template_ids = {"MT-01"};
    aoi.coordinates = {
        {0.0, 0.0, std::nullopt, std::nullopt},
        {0.0, 0.002, std::nullopt, std::nullopt},
        {0.002, 0.002, std::nullopt, std::nullopt},
        {0.002, 0.0, std::nullopt, std::nullopt}
    };
    mission.geometry_objects.push_back(aoi);

    request.camera.focal_length_mm = 12.29;
    request.camera.pixel_pitch_um = 3.3;
    request.camera.image_width_px = 5280;
    request.camera.image_height_px = 3956;
    request.camera.target_gsd_cm_px = 3.0;
    request.camera.forward_overlap_percent = 80.0;
    request.camera.side_overlap_percent = 70.0;
    request.camera.camera_configuration_id = "DJI-MAVIC-3E-WIDE-STANDARD";
    request.camera.calibration_source_reference =
        "DJI Mavic 3 Enterprise manufacturer specifications";
    request.camera.intrinsic_calibration_verified = false;
    request.minimum_altitude_agl_m = 50.0;
    request.maximum_altitude_agl_m = 150.0;
    request.cruise_speed_mps = 10.0;
    request.line_angle_deg = 0.0;
    return request;
}

int main()
{
    const auto request = makeRequest();
    const auto result = Mt01MappingPipeline::calculate(request);
    assert(result.status == Mt01PipelineStatus::CoverageCandidateAvailable);
    assert(result.coverage_candidate_available);
    assert(!result.flight_release_eligible);
    assert(result.coverage.valid);
    assert(result.coverage.metrics.coverage_segment_count > 0);
    assert(std::abs(result.coverage.metrics.calculated_altitude_agl_m - 111.7272727) < 0.01);

    bool blockedReleaseDisclosed = false;
    bool calibrationGapDisclosed = false;
    for (const auto& finding : result.findings) {
        if (finding.find("FLIGHT_RELEASE_BLOCKED") != std::string::npos)
            blockedReleaseDisclosed = true;
        if (finding.find("CAMERA_CALIBRATION_UNVERIFIED") != std::string::npos)
            calibrationGapDisclosed = true;
    }
    assert(blockedReleaseDisclosed);
    assert(calibrationGapDisclosed);

    auto missingVehicle = request;
    missingVehicle.mission.vehicle_assignments.clear();
    const auto noVehicle = Mt01MappingPipeline::calculate(missingVehicle);
    assert(noVehicle.status == Mt01PipelineStatus::NeedsInput);
    assert(!noVehicle.coverage_candidate_available);

    auto wrongMode = request;
    wrongMode.mission.mission_mode = "AUTOMATIC";
    assert(Mt01MappingPipeline::calculate(wrongMode).status == Mt01PipelineStatus::NeedsInput);

    auto missingAoi = request;
    missingAoi.mission.geometry_objects.clear();
    assert(Mt01MappingPipeline::calculate(missingAoi).status == Mt01PipelineStatus::NeedsInput);

    auto duplicateAoi = request;
    duplicateAoi.mission.geometry_objects.push_back(duplicateAoi.mission.geometry_objects.front());
    assert(Mt01MappingPipeline::calculate(duplicateAoi).status == Mt01PipelineStatus::NeedsInput);

    auto wrongTemplate = request;
    wrongTemplate.mission.selected_template_ids = {"MT-02"};
    assert(Mt01MappingPipeline::calculate(wrongTemplate).status == Mt01PipelineStatus::NeedsInput);

    return 0;
}
