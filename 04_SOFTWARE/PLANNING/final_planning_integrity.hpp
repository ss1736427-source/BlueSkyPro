#pragma once
#include "vehicle_mission_compiler.hpp"
#include <string>
#include <vector>
namespace bluesky::planning {
enum class FinalPlanningIntegrityStatus { Valid, Invalid };
enum class FinalPlanningIntegrityFindingCode {
    MissingMissionIdentity,
    MissingRouteVersion,
    MissingProfile,
    MissingVehicleBinding,
    MissingCalculationInput
};
struct FinalPlanningIntegrityFinding {
    FinalPlanningIntegrityFindingCode code;
    std::string detail;
};
struct FinalPlanningIntegrityResult {
    FinalPlanningIntegrityStatus status{FinalPlanningIntegrityStatus::Invalid};
    std::string dependency_identity;
    std::vector<FinalPlanningIntegrityFinding> findings;
};
class FinalPlanningIntegrityChecker {
public:
    static FinalPlanningIntegrityResult check(
        const VehicleCompiledProfile& compiled,
        const std::string& route_version,
        const std::string& calculation_input_version);
};
}
