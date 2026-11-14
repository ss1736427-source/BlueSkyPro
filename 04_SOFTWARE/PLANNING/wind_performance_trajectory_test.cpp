#include "wind_performance_trajectory.hpp"
#include <cassert>
#include <cmath>

using namespace bluesky::planning;

int main(){
    Route route;
    route.lineage.route_id="R1";
    route.lineage.route_version="1";
    route.lineage.calculation_input_version="ENV1";
    route.waypoints={
        {"A",{0.0,0.0},100.0,true},
        {"B",{0.0,0.001},100.0,true}
    };
    route.segments={{"S1","A","B",111.32,10.0}};

    TrajectoryPerformanceInput in;
    in.uav_id="U1";
    in.configuration_version="CFG1";
    in.performance_version="PERF1";
    in.wind_snapshot_id="W1";
    in.wind_snapshot_version="1";
    in.mission_start_epoch_s=1000;
    in.cruise_airspeed_mps=20.0;
    in.climb_rate_mps=2.0;
    in.descent_rate_mps=2.0;
    in.wind_tolerance_mps=15.0;
    in.energy_per_horizontal_meter_wh=0.01;
    in.payload_power_w=12.0;
    in.usable_energy_wh=100.0;
    in.reserve_requirement_wh=10.0;
    in.wind_samples={{"S1",0.0,5.0,0.0}};

    const auto r=WindPerformanceTrajectory::calculate(route,in,"CALC1");
    assert(r.status==TrajectoryStatus::Feasible);
    assert(r.mission_start_epoch_s==1000);
    assert(r.segments.size()==1);
    assert(r.points.size()==2);
    assert(r.segments[0].ground_speed_mps>20.0);
    assert(r.segments[0].traversal_time_s>4.0 && r.segments[0].traversal_time_s<5.0);
    assert(r.total_energy_wh>1.0);
    auto noPayload = in; noPayload.payload_power_w = 0.0;
    const auto noPayloadResult = WindPerformanceTrajectory::calculate(route, noPayload, "CALC1");
    assert(noPayloadResult.status == TrajectoryStatus::Feasible);
    assert(std::abs(r.total_energy_wh - noPayloadResult.total_energy_wh - in.payload_power_w*r.total_time_s/3600.0) < 1e-9);
    assert(r.remaining_energy_wh<100.0);
    assert(r.performance_version == in.performance_version);
    assert(r.usable_energy_wh == in.usable_energy_wh);
    assert(r.reserve_requirement_wh == in.reserve_requirement_wh);
    assert(!r.dependency_identity.empty());
    auto shiftedStart = in;
    shiftedStart.mission_start_epoch_s += 1;
    const auto shiftedResult =
        WindPerformanceTrajectory::calculate(route, shiftedStart, "CALC1");
    assert(shiftedResult.status == TrajectoryStatus::Feasible);
    assert(shiftedResult.dependency_identity != r.dependency_identity);
    auto changedReserve = in;
    changedReserve.reserve_requirement_wh += 1.0;
    const auto changedReserveResult =
        WindPerformanceTrajectory::calculate(route, changedReserve, "CALC1");
    assert(changedReserveResult.dependency_identity != r.dependency_identity);
    auto changedPayloadPower = in;
    changedPayloadPower.payload_power_w += 1.0;
    const auto changedPayloadResult = WindPerformanceTrajectory::calculate(route, changedPayloadPower, "CALC1");
    assert(changedPayloadResult.dependency_identity != r.dependency_identity);
    assert(changedPayloadResult.total_energy_wh > r.total_energy_wh);
    auto changedPerformance = in;
    changedPerformance.cruise_airspeed_mps += 1.0;
    const auto changedPerformanceResult =
        WindPerformanceTrajectory::calculate(route, changedPerformance, "CALC1");
    assert(changedPerformanceResult.dependency_identity != r.dependency_identity);

    Route withTaskDwell=route;
    withTaskDwell.waypoints.back().task_execution_duration_s=60.0;
    withTaskDwell.waypoints.back().task_execution_energy_wh=2.5;
    const auto dwellResult=WindPerformanceTrajectory::calculate(withTaskDwell,in,"CALC1");
    assert(dwellResult.status==TrajectoryStatus::Feasible);
    assert(std::abs(dwellResult.total_time_s-(r.total_time_s+60.0))<1e-9);
    assert(std::abs(dwellResult.total_energy_wh-(r.total_energy_wh+2.5))<1e-9);
    assert(std::abs(dwellResult.points.back().elapsed_time_s-dwellResult.total_time_s)<1e-9);

    Route withIntermediateTask=route;
    withIntermediateTask.waypoints.front().task_execution_duration_s=30.0;
    withIntermediateTask.waypoints.front().task_execution_energy_wh=1.0;
    const auto intermediateDwellResult=WindPerformanceTrajectory::calculate(
        withIntermediateTask,in,"CALC1");
    assert(intermediateDwellResult.status==TrajectoryStatus::Feasible);
    assert(std::abs(intermediateDwellResult.total_time_s-(r.total_time_s+30.0))<1e-9);
    assert(std::abs(intermediateDwellResult.total_energy_wh-(r.total_energy_wh+1.0))<1e-9);

    Route invalidTask=route;
    invalidTask.waypoints.front().task_execution_duration_s=30.0;
    const auto invalidTaskResult=WindPerformanceTrajectory::calculate(invalidTask,in,"CALC1");
    assert(invalidTaskResult.status==TrajectoryStatus::Infeasible);
    assert(!invalidTaskResult.findings.empty());

    auto cameraQualified = in;
    cameraQualified.camera_configuration_id = "CAM-01";
    cameraQualified.camera_gsd_m_px = 0.02;
    cameraQualified.camera_forward_trigger_spacing_m = 20.0;
    cameraQualified.camera_minimum_trigger_interval_s = 0.7;
    cameraQualified.camera_exposure_time_s = 1.0 / 2000.0;
    cameraQualified.camera_maximum_motion_blur_px = 1.0;
    cameraQualified.camera_capture_segment_ids = {"S1"};
    const auto cameraQualifiedResult =
        WindPerformanceTrajectory::calculate(route, cameraQualified, "CALC1");
    assert(cameraQualifiedResult.status == TrajectoryStatus::Feasible);
    assert(cameraQualifiedResult.segments[0].camera_trigger_interval_s >= 0.7);
    assert(cameraQualifiedResult.segments[0].camera_motion_blur_px <= 1.0);


    // Variable airspeed is integrated through acceleration, cruise and braking
    // phases. Wind is applied at each distance sample; camera limits use the
    // maximum resulting ground speed, not an average speed.
    auto variableProfileInput = cameraQualified;
    variableProfileInput.cruise_airspeed_mps = 0.0;
    variableProfileInput.speed_profile_id = "MT01-SP-01";
    variableProfileInput.speed_profile_version = "1";
    variableProfileInput.speed_profile_vehicle_model_id = "UAV-01";
    variableProfileInput.speed_profile_vehicle_model_version = "MODEL-1";
    variableProfileInput.speed_profile_configuration_id = "CFG1";
    variableProfileInput.speed_profile_speed_reference = "TAS";
    variableProfileInput.speed_profile_segments = {
        {"S1", 111.32, 10.0, 10.0, 18.0, 56.0, 37.3333333333,
         17.9866666667, 2.0, 3.0}
    };
    const auto variableProfileResult = WindPerformanceTrajectory::calculate(
        route, variableProfileInput, "CALC1");
    assert(variableProfileResult.status == TrajectoryStatus::Feasible);
    assert(variableProfileResult.segments[0].traversal_time_s > 111.32 / 23.0);
    assert(variableProfileResult.segments[0].ground_speed_mps > 15.0);
    assert(variableProfileResult.segments[0].camera_trigger_interval_s >= 0.7);
    assert(variableProfileResult.segments[0].camera_motion_blur_px <= 1.0);
    assert(variableProfileResult.dependency_identity != r.dependency_identity);

    auto incompleteProfile = variableProfileInput;
    incompleteProfile.speed_profile_segments.clear();
    assert(WindPerformanceTrajectory::calculate(route, incompleteProfile, "CALC1").status ==
           TrajectoryStatus::Feasible);
    auto mismatchedProfile = variableProfileInput;
    mismatchedProfile.speed_profile_segments[0].segment_id = "OTHER";
    assert(WindPerformanceTrajectory::calculate(route, mismatchedProfile, "CALC1").status ==
           TrajectoryStatus::Infeasible);
    auto cameraUnsafeProfile = variableProfileInput;
    cameraUnsafeProfile.speed_profile_segments[0].maximum_reachable_speed_mps = 25.0;
    cameraUnsafeProfile.speed_profile_segments[0].acceleration_distance_m = 100.0;
    cameraUnsafeProfile.speed_profile_segments[0].deceleration_distance_m = 50.0;
    cameraUnsafeProfile.speed_profile_segments[0].cruise_distance_m = -38.68;
    assert(WindPerformanceTrajectory::calculate(route, cameraUnsafeProfile, "CALC1").status ==
           TrajectoryStatus::Infeasible);

    Route routeWithConnector = route;
    routeWithConnector.waypoints.push_back({"C", {0.0, 0.002}, 100.0, true});
    routeWithConnector.segments.push_back({"S2", "B", "C", 111.32, 20.0});
    auto connectorWind = cameraQualified;
    connectorWind.wind_samples.push_back({"S2", 0.0, 10.0, 0.0});
    const auto connectorExcludedResult =
        WindPerformanceTrajectory::calculate(routeWithConnector, connectorWind, "CALC1");
    assert(connectorExcludedResult.status == TrajectoryStatus::Feasible);
    assert(connectorExcludedResult.segments.size() == 2);
    assert(connectorExcludedResult.segments[1].camera_trigger_interval_s == 0.0);
    assert(connectorExcludedResult.segments[1].camera_motion_blur_px == 0.0);

    auto cameraTriggerViolation = cameraQualified;
    cameraTriggerViolation.camera_forward_trigger_spacing_m = 10.0;
    const auto cameraTriggerViolationResult =
        WindPerformanceTrajectory::calculate(route, cameraTriggerViolation, "CALC1");
    assert(cameraTriggerViolationResult.status == TrajectoryStatus::Infeasible);
    assert(cameraTriggerViolationResult.findings.back().code ==
           TrajectoryFindingCode::CameraTriggerIntervalExceeded);

    auto cameraBlurViolation = cameraQualified;
    cameraBlurViolation.camera_exposure_time_s = 1.0 / 1000.0;
    const auto cameraBlurViolationResult =
        WindPerformanceTrajectory::calculate(route, cameraBlurViolation, "CALC1");
    assert(cameraBlurViolationResult.status == TrajectoryStatus::Infeasible);
    assert(cameraBlurViolationResult.findings.back().code ==
           TrajectoryFindingCode::CameraMotionBlurExceeded);

    auto incompleteCamera = cameraQualified;
    incompleteCamera.camera_capture_segment_ids.clear();
    assert(WindPerformanceTrajectory::calculate(route, incompleteCamera, "CALC1").status ==
           TrajectoryStatus::Infeasible);

    auto invalidPayload=in; invalidPayload.payload_power_w=-1.0;
    assert(WindPerformanceTrajectory::calculate(route,invalidPayload,"CALC1").status==TrajectoryStatus::Infeasible);

    auto missing=in; missing.wind_samples.clear();
    assert(WindPerformanceTrajectory::calculate(route,missing,"CALC1").status==TrajectoryStatus::Infeasible);

    auto excessive=in; excessive.wind_samples[0].north_mps=20.0;
    assert(WindPerformanceTrajectory::calculate(route,excessive,"CALC1").status==TrajectoryStatus::Infeasible);

    auto low_energy=in; low_energy.usable_energy_wh=1.0;
    assert(WindPerformanceTrajectory::calculate(route,low_energy,"CALC1").status==TrajectoryStatus::Infeasible);

    Route vertical=route;
    vertical.waypoints[1].position=vertical.waypoints[0].position;
    vertical.waypoints[1].altitude_m=120.0;
    vertical.segments[0].distance_m=0.0;
    auto vr=WindPerformanceTrajectory::calculate(vertical,in,"CALC1");
    assert(vr.status==TrajectoryStatus::Feasible);
    assert(vr.segments[0].ground_speed_mps==0.0);
    assert(vr.segments[0].traversal_time_s>9.9);

    return 0;
}
