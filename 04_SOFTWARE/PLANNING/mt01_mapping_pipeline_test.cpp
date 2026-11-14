#include "mt01_mapping_pipeline.hpp"

#include <cassert>
#include <cmath>
#include <cstdint>

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
    request.camera.minimum_trigger_interval_s = 0.7;
    request.camera.exposure_time_s = 1.0 / 2000.0;
    request.camera.maximum_motion_blur_px = 0.5;
    request.minimum_altitude_agl_m = 50.0;
    request.maximum_altitude_agl_m = 150.0;
    request.cruise_speed_mps = 10.0;
    request.line_angle_deg = 0.0;
    request.notam_snapshot.snapshot_id = "NOTAM-SNAPSHOT-TEST";
    request.notam_snapshot.source_id = "CONTROLLED-TEST-SOURCE";
    request.notam_snapshot.source_version = "1";
    request.notam_snapshot.evaluation_epoch_s = 1800000000;
    request.notam_snapshot.coverage_center = {0.001, 0.001};
    request.notam_snapshot.coverage_radius_m = 500.0;
    request.planned_mission_start_epoch_s = 1800000000;
    request.planned_mission_end_epoch_s = 1800003600;
    request.notam_safety_buffer_m = 25.0;
    return request;
}

int main()
{
    const auto request = makeRequest();

    // NOTAM snapshot is now a mandatory input to MT-01 geometry generation.
    auto missingNotam = request;
    missingNotam.notam_snapshot = NotamSnapshot{};
    const auto missingNotamResult = Mt01MappingPipeline::calculate(missingNotam);
    assert(missingNotamResult.status == Mt01PipelineStatus::NeedsInput);
    bool missingSnapshotReported = false;
    for (const auto& finding : missingNotamResult.findings)
        if (finding.find("NOTAM_SNAPSHOT_REQUIRED") != std::string::npos)
            missingSnapshotReported = true;
    assert(missingSnapshotReported);

    auto incompleteSourceCoverage = request;
    incompleteSourceCoverage.notam_snapshot.coverage_radius_m = 50.0;
    const auto incompleteCoverageResult =
        Mt01MappingPipeline::calculate(incompleteSourceCoverage);
    assert(incompleteCoverageResult.status == Mt01PipelineStatus::NeedsInput);
    bool sourceCoverageBlocked = false;
    for (const auto& finding : incompleteCoverageResult.findings)
        if (finding.find("NOTAM_SNAPSHOT_DOES_NOT_COVER_AOI_WITH_BUFFER") != std::string::npos)
            sourceCoverageBlocked = true;
    assert(sourceCoverageBlocked);

    // An applicable circular NOTAM must reduce coverage and bind its snapshot
    // identity to the generated canonical route candidate.
    auto withNotam = request;
    NotamZone circle;
    circle.notam_id = "TEST-CIRCLE-01";
    circle.component_id = "TEST-CIRCLE-01-COMP-01";
    circle.restriction_type = NotamRestrictionType::Prohibited;
    circle.geometry_type = NotamGeometryType::Circle;
    circle.center = {0.001, 0.001};
    circle.radius_m = 35.0;
    circle.lower_altitude_m = 50.0;
    circle.upper_altitude_m = 150.0;
    circle.valid_from_epoch_s = withNotam.planned_mission_start_epoch_s;
    circle.valid_to_epoch_s = withNotam.planned_mission_end_epoch_s;
    circle.altitude_reference = NotamAltitudeReference::AGL;
    withNotam.notam_snapshot.zones.push_back(circle);
    const auto clearResult = Mt01MappingPipeline::calculate(request);
    const auto restrictedResult = Mt01MappingPipeline::calculate(withNotam);
    assert(restrictedResult.notam_geometry.valid);
    assert(restrictedResult.notam_geometry.applicable_zone_count == 1);
    assert(!restrictedResult.notam_geometry.exclusion_areas.empty());
    assert(restrictedResult.coverage.valid);
    assert(restrictedResult.coverage.metrics.estimated_coverage_leg_length_m <
           clearResult.coverage.metrics.estimated_coverage_leg_length_m);
    assert(restrictedResult.route_candidate_available == false ||
           restrictedResult.route_candidate.route.environment.airspace_snapshot_id ==
               withNotam.notam_snapshot.snapshot_id);

    // A zone outside the planned time window is not used for initial geometry.
    auto futureNotam = request;
    auto futureZone = circle;
    futureZone.notam_id = "TEST-FUTURE-01";
    futureZone.valid_from_epoch_s = futureNotam.planned_mission_end_epoch_s + 3600;
    futureZone.valid_to_epoch_s = futureZone.valid_from_epoch_s + 3600;
    futureNotam.notam_snapshot.zones.push_back(futureZone);
    const auto futureResult = Mt01MappingPipeline::calculate(futureNotam);
    assert(futureResult.notam_geometry.valid);
    assert(futureResult.notam_geometry.applicable_zone_count == 0);

    // An active polygon is expanded by the configured buffer into a conservative
    // union of the source polygon, edge strips and circumscribed vertex disks.
    auto polygonNotam = request;
    NotamZone polygonZone;
    polygonZone.notam_id = "TEST-COMPOUND-01";
    polygonZone.component_id = "TEST-COMPOUND-01-AREA-01";
    polygonZone.restriction_type = NotamRestrictionType::Prohibited;
    polygonZone.geometry_type = NotamGeometryType::Polygon;
    polygonZone.polygon = {
        {0.0006, 0.0006}, {0.0006, 0.0014},
        {0.0014, 0.0014}, {0.0014, 0.0006}
    };
    polygonZone.lower_altitude_m = 50.0;
    polygonZone.upper_altitude_m = 150.0;
    polygonZone.valid_from_epoch_s = polygonNotam.planned_mission_start_epoch_s;
    polygonZone.valid_to_epoch_s = polygonNotam.planned_mission_end_epoch_s;
    polygonZone.altitude_reference = NotamAltitudeReference::AGL;
    polygonNotam.notam_snapshot.zones.push_back(polygonZone);
    auto circleComponent = circle;
    circleComponent.notam_id = "TEST-COMPOUND-01";
    circleComponent.component_id = "TEST-COMPOUND-01-WI-CIRCLE-01";
    polygonNotam.notam_snapshot.zones.push_back(circleComponent);
    const auto polygonResult = Mt01MappingPipeline::calculate(polygonNotam);
    assert(polygonResult.notam_geometry.valid);
    assert(polygonResult.notam_geometry.applicable_zone_count == 2);
    assert(polygonResult.notam_geometry.applicable_notam_ids.size() == 2);
    assert(polygonResult.notam_geometry.applicable_component_ids.size() == 2);
    assert(polygonResult.notam_geometry.applicable_component_ids[0] ==
           "TEST-COMPOUND-01-AREA-01");
    assert(polygonResult.notam_geometry.applicable_component_ids[1] ==
           "TEST-COMPOUND-01-WI-CIRCLE-01");
    assert(polygonResult.notam_geometry.exclusion_areas.size() > 1);
    assert(polygonResult.coverage.valid);
    assert(polygonResult.coverage.metrics.estimated_coverage_leg_length_m <
           clearResult.coverage.metrics.estimated_coverage_leg_length_m);

    auto missingBuffer = withNotam;
    missingBuffer.notam_safety_buffer_m = 0.0;
    const auto missingBufferResult = Mt01MappingPipeline::calculate(missingBuffer);
    assert(missingBufferResult.status == Mt01PipelineStatus::NeedsInput);
    bool bufferRequirementReported = false;
    for (const auto& finding : missingBufferResult.findings)
        if (finding.find("NOTAM_SAFETY_BUFFER_REQUIRED") != std::string::npos)
            bufferRequirementReported = true;
    assert(bufferRequirementReported);

    // An applicable NOTAM with an unsupported vertical datum must fail closed.
    auto unknownDatum = withNotam;
    unknownDatum.notam_snapshot.zones.front().altitude_reference =
        NotamAltitudeReference::Unknown;
    const auto unknownDatumResult = Mt01MappingPipeline::calculate(unknownDatum);
    assert(unknownDatumResult.status == Mt01PipelineStatus::NeedsInput);

    const auto result = Mt01MappingPipeline::calculate(request);
    assert(result.status == Mt01PipelineStatus::CoverageCandidateAvailable);
    assert(result.coverage_candidate_available);
    assert(!result.flight_release_eligible);
    assert(!result.kinematic_geometry_available);
    assert(result.coverage.valid);
    assert(result.coverage.metrics.coverage_segment_count > 0);
    assert(std::abs(result.coverage.metrics.calculated_altitude_agl_m - 111.7272727) < 0.01);
    assert(result.coverage.metrics.nominal_trigger_interval_s > 0.7);
    assert(result.coverage.metrics.nominal_motion_blur_px < 0.5);


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
    fixedWingRequest.requested_max_turn_radius_m = 15.0;
    fixedWingRequest.fixed_wing_clean_stall_speed_mps = 6.0;
    fixedWingRequest.fixed_wing_stall_speed_reference = "TAS";
    fixedWingRequest.fixed_wing_stall_margin_factor = 1.20;
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
    auto& routeMetadata = fixedWingRequest.route_candidate_metadata;
    routeMetadata.route_id = "MT01-ROUTE-PIPELINE-TEST";
    routeMetadata.route_version = "1";
    routeMetadata.generator_id = "MT01-KINEMATIC-GENERATOR";
    routeMetadata.generator_version = "1";
    routeMetadata.calculation_input_version = "INPUT-PIPELINE-TEST-1";
    routeMetadata.constraints = {50.0, 150.0, 30.0, true};
    const auto fixedWingResult = Mt01MappingPipeline::calculate(fixedWingRequest);
    assert(fixedWingResult.coverage_candidate_available);
    assert(fixedWingResult.kinematic_geometry_available);
    assert(fixedWingResult.turn_speed_constraint_applied);
    assert(fixedWingResult.selected_connector_speed_mps < 10.0);
    assert(fixedWingResult.minimum_safe_connector_speed_mps > 5.0);
    assert(std::abs(fixedWingResult.kinematic_geometry.minimum_turn_radius_m
                    - 15.0) < 1e-6);
    for (const auto& segment : fixedWingResult.kinematic_geometry.segments) {
        if (segment.type == Mt01KinematicSegmentType::ConnectorTurn) {
            assert(std::abs(segment.nominal_speed_mps
                            - fixedWingResult.selected_connector_speed_mps) < 1e-9);
            assert(std::abs(segment.minimum_speed_mps
                            - segment.nominal_speed_mps) < 1e-9);
        }
    }

    assert(fixedWingResult.kinematic_geometry.segments.size() + 1 ==
           fixedWingResult.kinematic_geometry.waypoints.size());
    assert(fixedWingResult.segment_dynamics_validated);
    assert(fixedWingResult.flight_safety_validation.status == FlightSafetyStatus::Allowed);
    assert(fixedWingResult.route_candidate_available);
    assert(fixedWingResult.route_candidate.route.environment.airspace_snapshot_id ==
           fixedWingRequest.notam_snapshot.snapshot_id);
    assert(fixedWingResult.route_candidate.structural_validation.status ==
           RouteValidationStatus::Allowed);

    auto cameraDwellEvents = fixedWingRequest;
    cameraDwellEvents.task_execution_events.push_back(Mt01TaskExecutionEvent{});
    const auto rejectedCameraDwell = Mt01MappingPipeline::calculate(cameraDwellEvents);
    bool cameraDwellRejected = false;
    for (const auto& finding : rejectedCameraDwell.findings)
        if (finding.find("MT01_CAMERA_EXPOSURE_DWELL_MODEL_REJECTED") != std::string::npos)
            cameraDwellRejected = true;
    assert(cameraDwellRejected);
    assert(!rejectedCameraDwell.task_execution_events_applied);
    assert(!fixedWingResult.route_candidate.route.trajectory_geometry_complete);
    assert(!fixedWingResult.flight_release_eligible);
    auto missingStallEvidence = fixedWingRequest;
    missingStallEvidence.fixed_wing_clean_stall_speed_mps = 0.0;
    const auto missingStallResult = Mt01MappingPipeline::calculate(missingStallEvidence);
    assert(missingStallResult.coverage_candidate_available);
    assert(!missingStallResult.kinematic_geometry_available);
    assert(!missingStallResult.turn_speed_constraint_applied);
    bool missingStallBlocked = false;
    for (const auto& finding : missingStallResult.findings)
        if (finding.find("MT01_TURN_SPEED_CAP_UNAVAILABLE") != std::string::npos)
            missingStallBlocked = true;
    assert(missingStallBlocked);


    auto missingRouteProvenance = fixedWingRequest;
    missingRouteProvenance.route_candidate_metadata.calculation_input_version.clear();
    const auto missingProvenanceResult =
        Mt01MappingPipeline::calculate(missingRouteProvenance);
    assert(missingProvenanceResult.kinematic_geometry_available);
    assert(missingProvenanceResult.segment_dynamics_validated);
    assert(!missingProvenanceResult.route_candidate_available);
    assert(!missingProvenanceResult.flight_release_eligible);

    return 0;
}
