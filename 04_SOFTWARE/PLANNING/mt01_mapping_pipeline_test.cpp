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
    mission.vehicle_assignments.push_back({"DJI-MAVIC-3E", "MAVIC-3E-STANDARD-REFERENCE-001", "MAVIC-3E-STANDARD", "STANDARD", "APPROVED", "APPROVED"});
    mission.equipment_assignments.push_back({"DJI-MAVIC-3E", "DJI-MAVIC-3E-WIDE", "DJI-MAVIC-3E-WIDE-PROFILE", "CAMERA", "MAVIC-3E-WIDE-REFERENCE-001", "COMPATIBLE", "VERIFIED", "NOT_VERIFIED"});

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
    request.camera.camera_configuration_id = "DJI-MAVIC-3E-WIDE";
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
    assert(!result.kinematic_geometry_available);
    assert(result.coverage.valid);
    assert(result.coverage.metrics.coverage_segment_count > 0);
    assert(std::abs(result.coverage.metrics.calculated_altitude_agl_m - 111.7272727) < 0.01);

    bool blockedReleaseDisclosed = false;
    bool calibrationGapDisclosed = false;
    bool kinematicBlockDisclosed = false;
    for (const auto& finding : result.findings) {
        if (finding.find("FLIGHT_RELEASE_BLOCKED") != std::string::npos)
            blockedReleaseDisclosed = true;
        if (finding.find("CAMERA_CALIBRATION_UNVERIFIED") != std::string::npos)
            calibrationGapDisclosed = true;
        if (finding.find("MT01_KINEMATIC_GEOMETRY_BLOCKED") != std::string::npos)
            kinematicBlockDisclosed = true;
    }
    assert(blockedReleaseDisclosed);
    assert(calibrationGapDisclosed);
    assert(kinematicBlockDisclosed);

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

    auto mismatchedCamera = request;
    mismatchedCamera.camera.camera_configuration_id = "UNASSIGNED-CAMERA";
    const auto cameraMismatch = Mt01MappingPipeline::calculate(mismatchedCamera);
    assert(cameraMismatch.status == Mt01PipelineStatus::NeedsInput);
    bool mismatchReported = false;
    for (const auto& finding : cameraMismatch.findings)
        if (finding == "CAMERA_ASSIGNMENT_MISMATCH") mismatchReported = true;
    assert(mismatchReported);

    // CUSTOM derived configurations are accepted for analysis only when the
    // exact equipment assignment has confirmed compatibility and verification.
    auto customConfiguration = request;
    customConfiguration.mission.vehicle_assignments.front().configuration_id = "MAVIC-3E-DERIVED-CUSTOM-01";
    customConfiguration.mission.vehicle_assignments.front().configuration_version = "2";
    customConfiguration.mission.vehicle_assignments.front().configuration_type = "CUSTOM";
    customConfiguration.mission.vehicle_assignments.front().validation_state = "VALIDATED";
    customConfiguration.mission.vehicle_assignments.front().approval_state = "PENDING";
    const auto customResult = Mt01MappingPipeline::calculate(customConfiguration);
    assert(customResult.status == Mt01PipelineStatus::CoverageCandidateAvailable);
    assert(!customResult.flight_release_eligible);

    auto unknownCompatibility = customConfiguration;
    unknownCompatibility.mission.equipment_assignments.front().compatibility_state = "UNKNOWN";
    const auto unknownResult = Mt01MappingPipeline::calculate(unknownCompatibility);
    assert(unknownResult.status == Mt01PipelineStatus::NeedsInput);
    bool compatibilityBlocked = false;
    for (const auto& finding : unknownResult.findings)
        if (finding == "CAMERA_COMPATIBILITY_NOT_CONFIRMED") compatibilityBlocked = true;
    assert(compatibilityBlocked);

    auto draftCustomConfiguration = customConfiguration;
    draftCustomConfiguration.mission.vehicle_assignments.front().validation_state = "DRAFT";
    const auto draftConfigurationResult = Mt01MappingPipeline::calculate(draftCustomConfiguration);
    assert(draftConfigurationResult.status == Mt01PipelineStatus::NeedsInput);
    bool configurationBlocked = false;
    for (const auto& finding : draftConfigurationResult.findings)
        if (finding == "VEHICLE_CONFIGURATION_NOT_VALIDATED") configurationBlocked = true;
    assert(configurationBlocked);


    auto wrongVehicleBinding = request;
    wrongVehicleBinding.mission.equipment_assignments.front().vehicle_id = "OTHER-UAV";
    assert(Mt01MappingPipeline::calculate(wrongVehicleBinding).status == Mt01PipelineStatus::NeedsInput);

    // Full integration fixture: fixed-wing kinematic geometry is checked against
    // an independently supplied, exact-configuration safety envelope.
    auto fixedWingRequest = request;
    auto& assignment = fixedWingRequest.mission.vehicle_assignments.front();
    assignment.vehicle_id = "TEST-FIXED-WING";
    assignment.configuration_id = "TEST-FIXED-WING-CFG";
    assignment.configuration_version = "fixture-1";
    fixedWingRequest.mission.equipment_assignments.front().vehicle_id = assignment.vehicle_id;
    fixedWingRequest.vehicle_motion_model.model_id = assignment.vehicle_id;
    fixedWingRequest.vehicle_motion_model.model_version = assignment.configuration_version;
    fixedWingRequest.vehicle_motion_model.configuration_id = assignment.configuration_id;
    fixedWingRequest.vehicle_motion_model.motion_class = MotionClass::FixedWing;
    fixedWingRequest.vehicle_motion_model.evidence = MotionEvidence::FlightTestValidated;
    fixedWingRequest.vehicle_motion_model.reference_speed_mps = 10.0;
    fixedWingRequest.vehicle_motion_model.minimum_speed_mps = 5.0;
    fixedWingRequest.vehicle_motion_model.maximum_speed_mps = 20.0;
    fixedWingRequest.vehicle_motion_model.bank_angle_rad =
        30.0 * 3.14159265358979323846 / 180.0;
    fixedWingRequest.vehicle_motion_model.speed_reference = "TAS";
    fixedWingRequest.vehicle_motion_model.source_reference = "controlled integration fixture";
    auto& safety = fixedWingRequest.flight_safety_envelope;
    safety.vehicle_model_id = assignment.vehicle_id;
    safety.model_version = assignment.configuration_version;
    safety.configuration_id = assignment.configuration_id;
    safety.source_reference = "controlled integration fixture";
    safety.speed_reference = "TAS";
    safety.minimum_speed_basis =
        "CONFIGURATION_SPECIFIC_STALL_SPEED_WITH_BANK_AND_MARGIN";
    safety.evidence = FlightSafetyEvidence::FlightTestValidated;
    safety.motion_class = MotionClass::FixedWing;
    safety.minimum_speed_mps = 5.0;
    safety.maximum_speed_mps = 20.0;
    safety.maximum_bank_angle_rad =
        fixedWingRequest.vehicle_motion_model.bank_angle_rad;
    safety.maximum_lateral_acceleration_mps2 = 6.0;
    const auto fixedWingResult = Mt01MappingPipeline::calculate(fixedWingRequest);
    assert(fixedWingResult.coverage_candidate_available);
    assert(fixedWingResult.kinematic_geometry_available);
    assert(fixedWingResult.kinematic_geometry.segments.size() + 1 ==
           fixedWingResult.kinematic_geometry.waypoints.size());
    assert(fixedWingResult.segment_dynamics_validated);
    assert(fixedWingResult.flight_safety_validation.status == FlightSafetyStatus::Allowed);
    assert(!fixedWingResult.flight_release_eligible);

    return 0;
}
