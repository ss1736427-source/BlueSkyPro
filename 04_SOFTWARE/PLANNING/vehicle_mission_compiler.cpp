#include "vehicle_mission_compiler.hpp"
#include <cmath>

namespace bluesky::planning {

VehicleCompiledProfile VehicleMissionCompiler::compile(
    const FlightProfile& profile, const VehicleCompilationInput& input) {
    VehicleCompiledProfile r;
    r.mission_id = profile.mission_id;
    r.mission_version = profile.mission_version;
    r.uav_id = input.uav_id;
    r.configuration_version = input.configuration_version;
    r.performance_version = input.performance_version;
    r.source_profile_id = profile.profile_id;
    r.route_version = profile.route_version;
    r.calculation_input_version = profile.calculation_input_version;
    r.dependency_identity = profile.dependency_identity + "|" + input.uav_id + "|" +
        input.configuration_version + "|" + input.performance_version;

    if (!profile.valid || profile.mission_id.empty() || profile.mission_version.empty() ||
        profile.profile_id.empty() || profile.profile_version.empty() ||
        profile.source_candidate_id.empty() || profile.route_version.empty() ||
        profile.calculation_input_version.empty() || profile.dependency_identity.empty() ||
        profile.points.size() < 2) {
        r.findings.push_back({VehicleCompilationFindingCode::InvalidFlightProfile,
                              "Flight profile is invalid or lacks required lineage/points"});
        return r;
    }

    if (input.uav_id.empty() || input.configuration_version.empty() ||
        input.performance_version.empty()) {
        r.findings.push_back({VehicleCompilationFindingCode::InvalidVehicleIdentity,
                              "Vehicle identity/configuration/performance version is incomplete"});
        return r;
    }

    if (!std::isfinite(input.maximum_altitude_m) || input.maximum_altitude_m < 0.0 ||
        !std::isfinite(input.minimum_speed_mps) || input.minimum_speed_mps < 0.0 ||
        !std::isfinite(input.maximum_speed_mps) || input.maximum_speed_mps <= 0.0 ||
        !std::isfinite(input.cruise_speed_mps) ||
        input.minimum_speed_mps > input.maximum_speed_mps ||
        input.cruise_speed_mps < input.minimum_speed_mps ||
        input.cruise_speed_mps > input.maximum_speed_mps) {
        r.findings.push_back({VehicleCompilationFindingCode::SpeedLimitInvalid,
                              "Invalid vehicle altitude/speed limits"});
        return r;
    }

    double previous_time = -1.0;
    for (std::size_t i = 0; i < profile.points.size(); ++i) {
        const auto& p = profile.points[i];
        if (!std::isfinite(p.position.latitude_deg) ||
            !std::isfinite(p.position.longitude_deg) ||
            !std::isfinite(p.altitude_m) ||
            !std::isfinite(p.cumulative_time_s) ||
            p.cumulative_time_s < 0.0 || p.cumulative_time_s < previous_time ||
            (i == 0 && std::abs(p.cumulative_time_s) > 1e-6)) {
            r.findings.push_back({VehicleCompilationFindingCode::InvalidFlightProfile,
                                  "Flight profile contains invalid geometry or timing"});
            return r;
        }
        if (p.altitude_m > input.maximum_altitude_m) {
            r.findings.push_back({VehicleCompilationFindingCode::AltitudeLimitExceeded,
                                  "Flight profile altitude exceeds vehicle maximum"});
            return r;
        }
        previous_time = p.cumulative_time_s;
    }

    r.points = profile.points;
    r.status = VehicleCompilationStatus::Compiled;
    return r;
}

}
