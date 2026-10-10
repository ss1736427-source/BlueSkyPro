#include "final_planning_integrity.hpp"
namespace bluesky::planning {
FinalPlanningIntegrityResult FinalPlanningIntegrityChecker::check(
    const VehicleCompiledProfile& c, const std::string& route_version,
    const std::string& calculation_input_version) {
    FinalPlanningIntegrityResult r;
    r.dependency_identity=c.dependency_identity+"|"+route_version+"|"+calculation_input_version;
    if(c.mission_id.empty()||c.mission_version.empty())
        r.findings.push_back({FinalPlanningIntegrityFindingCode::MissingMissionIdentity,"Mission identity is incomplete"});
    if(route_version.empty())
        r.findings.push_back({FinalPlanningIntegrityFindingCode::MissingRouteVersion,"Route version is missing"});
    if(c.source_profile_id.empty())
        r.findings.push_back({FinalPlanningIntegrityFindingCode::MissingProfile,"Flight profile reference is missing"});
    if(c.uav_id.empty()||c.configuration_version.empty()||c.performance_version.empty())
        r.findings.push_back({FinalPlanningIntegrityFindingCode::MissingVehicleBinding,"Vehicle binding is incomplete"});
    if(calculation_input_version.empty())
        r.findings.push_back({FinalPlanningIntegrityFindingCode::MissingCalculationInput,"Calculation input version is missing"});
    if(c.status!=VehicleCompilationStatus::Compiled)
        r.findings.push_back({FinalPlanningIntegrityFindingCode::MissingVehicleBinding,"Vehicle compilation is not feasible"});
    r.status=r.findings.empty()?FinalPlanningIntegrityStatus::Valid:FinalPlanningIntegrityStatus::Invalid;
    return r;
}
}
