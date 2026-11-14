#include "mt01_weather_performance_adapter.hpp"

#include <cassert>
#include <cmath>
#include <limits>

using namespace bluesky::planning;

int main()
{
    Mission mission;
    mission.vehicle_assignments.push_back({"UAV-01","AIRFRAME-V2","CFG-02","STANDARD","VALIDATED","APPROVED"});
    mission.equipment_assignments.push_back({"UAV-01","CAM-01","CAM-PROFILE-01","CAMERA","CAM-V3","COMPATIBLE","VERIFIED","VERIFIED"});
    mission.equipment_assignments.push_back({"UAV-01","LIDAR-01","LIDAR-PROFILE-01","LIDAR","LIDAR-V1","COMPATIBLE","VERIFIED","VERIFIED"});
    VehicleEquipmentCapabilityProfile capability;
    capability.identity.vehicle_id="UAV-01";
    capability.configuration.configuration_version="AIRFRAME-V2";
    capability.configuration.valid_for_mission=true;
    EquipmentCapability cam; cam.equipment_id="CAM-01"; cam.equipment_version="CAM-V3";
    cam.sensor_camera_type="RGB"; cam.electrical_power=18.0; cam.triggering_capable=true;
    cam.equipment_profile_id="CAM-PROFILE-01";
    EquipmentCapability lidar; lidar.equipment_id="LIDAR-01"; lidar.equipment_version="LIDAR-V1";
    lidar.electrical_power=22.0; lidar.equipment_profile_id="LIDAR-PROFILE-01";
    capability.configuration.equipment={cam,lidar};

    Mt01MappingPipelineResult pipeline;
    pipeline.coverage_candidate_available=true; pipeline.kinematic_geometry_available=true;
    pipeline.route_candidate_available=true; pipeline.coverage.valid=true;
    pipeline.coverage.metrics.forward_trigger_spacing_m=12.0;
    pipeline.kinematic_geometry.segments={
        {"COV-01",0,1,Mt01KinematicSegmentType::Coverage,100.0,12.0,0.0,0.0},
        {"TURN-01",1,2,Mt01KinematicSegmentType::ConnectorTurn,20.0,12.0,0.3,2.0}};
    pipeline.route_candidate.available=true;
    pipeline.route_candidate.route.segments={
        {"COV-01","A","B",100.0,12.0},{"TURN-01","B","C",20.0,12.0}};
    CoverageCameraModel camera; camera.camera_configuration_id="CAM-01";
    camera.calibration_source_reference="manufacturer reference"; camera.target_gsd_cm_px=2.0;
    camera.minimum_trigger_interval_s=0.7; camera.exposure_time_s=1.0/2000.0;
    camera.maximum_motion_blur_px=0.5;
    TrajectoryPerformanceInput base; base.uav_id="UAV-01";
    base.configuration_version="AIRFRAME-V2"; base.performance_version="PERF-07";
    const auto bound=Mt01WeatherPerformanceAdapter::bind(mission,pipeline,camera,capability,base);
    assert(bound.valid);
    assert(std::abs(bound.performance.payload_power_w-40.0)<1e-9);
    assert(bound.performance.camera_configuration_id=="CAM-01");
    assert(std::abs(bound.performance.camera_gsd_m_px-0.02)<1e-9);
    assert(bound.performance.camera_capture_segment_ids==std::vector<std::string>{"COV-01"});

    auto wrongVehicle=capability; wrongVehicle.identity.vehicle_id="OTHER";
    assert(!Mt01WeatherPerformanceAdapter::bind(mission,pipeline,camera,wrongVehicle,base).valid);
    auto wrongEquipmentProfile=capability;
    wrongEquipmentProfile.configuration.equipment.front().equipment_profile_id="OTHER-PROFILE";
    assert(!Mt01WeatherPerformanceAdapter::bind(
        mission,pipeline,camera,wrongEquipmentProfile,base).valid);
    auto badPower=capability; badPower.configuration.equipment.front().electrical_power=std::numeric_limits<double>::quiet_NaN();
    assert(!Mt01WeatherPerformanceAdapter::bind(mission,pipeline,camera,badPower,base).valid);
    auto noTrigger=capability; noTrigger.configuration.equipment.front().triggering_capable=false;
    assert(!Mt01WeatherPerformanceAdapter::bind(mission,pipeline,camera,noTrigger,base).valid);
    auto unverified=mission; unverified.equipment_assignments.front().verification_state="NOT_VERIFIED";
    assert(!Mt01WeatherPerformanceAdapter::bind(unverified,pipeline,camera,capability,base).valid);
    return 0;
}
