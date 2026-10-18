#include "final_planning_integrity.hpp"
#include <cmath>

namespace bluesky::planning {

FinalPlanningIntegrityResult FinalPlanningIntegrityChecker::check(
    const VehicleCompiledProfile& c, const std::string& route_version,
    const std::string& calculation_input_version) {
    FinalPlanningIntegrityResult r;
    r.dependency_identity = c.dependency_identity + "|route:" + route_version +
        "|input:" + calculation_input_version;

    if (c.mission_id.empty() || c.mission_version.empty())
        r.findings.push_back({FinalPlanningIntegrityFindingCode::MissingMissionIdentity,
                              "Mission identity is incomplete"});

    if (route_version.empty() || c.route_version.empty())
        r.findings.push_back({FinalPlanningIntegrityFindingCode::MissingRouteVersion,
                              "Route version is missing"});
    else if (c.route_version != route_version)
        r.findings.push_back({FinalPlanningIntegrityFindingCode::RouteVersionMismatch,
                              "Compiled route version does not match final route version"});

    if (c.source_profile_id.empty() || c.dependency_identity.empty())
        r.findings.push_back({FinalPlanningIntegrityFindingCode::MissingProfile,
                              "Flight profile reference/dependency identity is missing"});

    if (c.uav_id.empty() || c.configuration_version.empty() || c.performance_version.empty())
        r.findings.push_back({FinalPlanningIntegrityFindingCode::MissingVehicleBinding,
                              "Vehicle binding is incomplete"});

    if (calculation_input_version.empty() || c.calculation_input_version.empty())
        r.findings.push_back({FinalPlanningIntegrityFindingCode::MissingCalculationInput,
                              "Calculation input version is missing"});
    else if (c.calculation_input_version != calculation_input_version)
        r.findings.push_back({FinalPlanningIntegrityFindingCode::CalculationInputMismatch,
                              "Compiled calculation-input version does not match final input version"});

    if (c.status != VehicleCompilationStatus::Compiled)
        r.findings.push_back({FinalPlanningIntegrityFindingCode::VehicleCompilationNotFeasible,
                              "Vehicle compilation is not feasible"});

    if (c.points.size() < 2) {
        r.findings.push_back({FinalPlanningIntegrityFindingCode::InvalidCompiledPoints,
                              "Compiled profile must contain at least two points"});
    } else {
        double previous_time = -1.0;
        for (std::size_t i = 0; i < c.points.size(); ++i) {
            const auto& p = c.points[i];
            if (!std::isfinite(p.position.latitude_deg) ||
                !std::isfinite(p.position.longitude_deg) ||
                !std::isfinite(p.altitude_m) ||
                !std::isfinite(p.cumulative_time_s) ||
                p.cumulative_time_s < 0.0 || p.cumulative_time_s < previous_time ||
                (i == 0 && std::abs(p.cumulative_time_s) > 1e-6)) {
                r.findings.push_back({FinalPlanningIntegrityFindingCode::InvalidCompiledPoints,
                                      "Compiled profile contains invalid geometry or timing"});
                break;
            }
            previous_time = p.cumulative_time_s;
        }
    }

    r.status = r.findings.empty() ? FinalPlanningIntegrityStatus::Valid
                                  : FinalPlanningIntegrityStatus::Invalid;
    return r;
}

}
