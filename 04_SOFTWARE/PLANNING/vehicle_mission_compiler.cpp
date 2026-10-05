#include "vehicle_mission_compiler.hpp"
namespace bluesky::planning {
VehicleCompiledProfile VehicleMissionCompiler::compile(
    const FlightProfile& profile, const VehicleCompilationInput& input) {
    VehicleCompiledProfile r;
    r.mission_id=profile.mission_id; r.mission_version=profile.mission_version;
    r.uav_id=input.uav_id; r.configuration_version=input.configuration_version;
    r.performance_version=input.performance_version; r.source_profile_id=profile.profile_id;
    r.dependency_identity=profile.dependency_identity+"|"+input.uav_id+"|"+input.configuration_version+"|"+input.performance_version;
    if (input.maximum_altitude_m < 0.0 || input.minimum_speed_mps < 0.0 ||
        input.maximum_speed_mps <= 0.0 || input.minimum_speed_mps > input.maximum_speed_mps ||
        input.cruise_speed_mps < input.minimum_speed_mps ||
        input.cruise_speed_mps > input.maximum_speed_mps) {
        r.findings.push_back({VehicleCompilationFindingCode::SpeedLimitInvalid,"Invalid vehicle speed limits"});
        return r;
    }
    for (const auto& p : profile.points) {
        if (p.altitude_m > input.maximum_altitude_m) {
            r.findings.push_back({VehicleCompilationFindingCode::AltitudeLimitExceeded,
                                  "Flight profile altitude exceeds vehicle maximum"});
            return r;
        }
    }
    r.points=profile.points;
    r.status=VehicleCompilationStatus::Compiled;
    return r;
}
}
