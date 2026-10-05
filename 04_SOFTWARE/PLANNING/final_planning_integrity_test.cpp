#include "final_planning_integrity.hpp"
#include <cassert>
using namespace bluesky::planning;
int main(){
 VehicleCompiledProfile c; c.status=VehicleCompilationStatus::Compiled;
 c.mission_id="M"; c.mission_version="1"; c.source_profile_id="FP";
 c.uav_id="U1"; c.configuration_version="C1"; c.performance_version="P1";
 c.dependency_identity="D";
 auto r=FinalPlanningIntegrityChecker::check(c,"R4","ENV3");
 assert(r.status==FinalPlanningIntegrityStatus::Valid);
 c.configuration_version.clear();
 r=FinalPlanningIntegrityChecker::check(c,"R4","ENV3");
 assert(r.status==FinalPlanningIntegrityStatus::Invalid);
 return 0;
}
