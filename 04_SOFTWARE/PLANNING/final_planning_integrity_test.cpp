#include "final_planning_integrity.hpp"
#include <cassert>
#include <limits>

using namespace bluesky::planning;

int main() {
    VehicleCompiledProfile c;
    c.status = VehicleCompilationStatus::Compiled;
    c.mission_id = "M";
    c.mission_version = "1";
    c.uav_id = "U1";
    c.configuration_version = "C1";
    c.performance_version = "P1";
    c.source_profile_id = "FP";
    c.route_version = "R4";
    c.calculation_input_version = "ENV3";
    c.dependency_identity = "D";
    c.points = {
        {{0.0, 0.0}, 50.0, 0.0, true},
        {{0.0, 0.001}, 60.0, 10.0, true}
    };

    auto r = FinalPlanningIntegrityChecker::check(c, "R4", "ENV3");
    assert(r.status == FinalPlanningIntegrityStatus::Valid);

    c.configuration_version.clear();
    r = FinalPlanningIntegrityChecker::check(c, "R4", "ENV3");
    assert(r.status == FinalPlanningIntegrityStatus::Invalid);

    c.configuration_version = "C1";
    r = FinalPlanningIntegrityChecker::check(c, "R5", "ENV3");
    assert(r.status == FinalPlanningIntegrityStatus::Invalid);

    r = FinalPlanningIntegrityChecker::check(c, "R4", "ENV4");
    assert(r.status == FinalPlanningIntegrityStatus::Invalid);

    auto empty_points = c;
    empty_points.points.clear();
    r = FinalPlanningIntegrityChecker::check(empty_points, "R4", "ENV3");
    assert(r.status == FinalPlanningIntegrityStatus::Invalid);

    auto invalid_time = c;
    invalid_time.points[1].cumulative_time_s = std::numeric_limits<double>::quiet_NaN();
    r = FinalPlanningIntegrityChecker::check(invalid_time, "R4", "ENV3");
    assert(r.status == FinalPlanningIntegrityStatus::Invalid);

    c.status = VehicleCompilationStatus::Infeasible;
    r = FinalPlanningIntegrityChecker::check(c, "R4", "ENV3");
    assert(r.status == FinalPlanningIntegrityStatus::Invalid);
    return 0;
}
