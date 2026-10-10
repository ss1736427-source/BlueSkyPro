#pragma once
#include "flight_profile.hpp"
#include <string>
namespace bluesky::planning {
struct VehicleCompilationInput {
    std::string uav_id;
    std::string configuration_version;
    std::string performance_version;
    double maximum_altitude_m{0.0};
    double minimum_speed_mps{0.0};
    double maximum_speed_mps{0.0};
    double cruise_speed_mps{0.0};
};
enum class VehicleCompilationStatus { Compiled, Infeasible };
enum class VehicleCompilationFindingCode { AltitudeLimitExceeded, SpeedLimitInvalid };
struct VehicleCompilationFinding { VehicleCompilationFindingCode code; std::string detail; };
struct VehicleCompiledProfile {
    std::string mission_id;
    std::string mission_version;
    std::string uav_id;
    std::string configuration_version;
    std::string performance_version;
    std::string source_profile_id;
    std::string dependency_identity;
    VehicleCompilationStatus status{VehicleCompilationStatus::Infeasible};
    std::vector<FlightProfilePoint> points;
    std::vector<VehicleCompilationFinding> findings;
};
class VehicleMissionCompiler {
public:
    static VehicleCompiledProfile compile(const FlightProfile& profile,
                                          const VehicleCompilationInput& input);
};
}
