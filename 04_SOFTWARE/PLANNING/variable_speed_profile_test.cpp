#include "variable_speed_profile.hpp"
#include <cassert>
using namespace bluesky::planning;
int main() {
 SpeedProfileInput in; in.vehicle_model_id="UAV-01"; in.vehicle_model_version="1"; in.configuration_id="CFG-01";
 in.speed_reference="TAS"; in.longitudinal_dynamics_validated=true; in.longitudinal_dynamics_source="flight-test:LONG-DYN-01";
 in.maximum_acceleration_mps2=2.0; in.maximum_deceleration_mps2=3.0; in.initial_speed_mps=20.0; in.final_speed_mps=20.0;
 in.segments={{"COVER-1",300.0,12.0,25.0},{"TURN-1",80.0,12.0,16.0},{"COVER-2",300.0,12.0,25.0}};
 const auto p=VariableSpeedProfileSolver::calculate(in); assert(p.status==SpeedProfileStatus::Feasible); assert(p.segments.size()==3);
 assert(p.segments[1].maximum_reachable_speed_mps<=16.0+1e-9);
 assert(p.segments[0].exit_speed_mps==p.segments[1].entry_speed_mps);
 assert(p.segments[1].acceleration_distance_m>=0.0&&p.segments[1].deceleration_distance_m>=0.0&&p.segments[1].cruise_distance_m>=0.0);
 auto missing=in; missing.longitudinal_dynamics_validated=false; assert(VariableSpeedProfileSolver::calculate(missing).status==SpeedProfileStatus::Infeasible);
 auto shortLeg=in; shortLeg.segments[0].distance_m=1.0; shortLeg.segments[1].minimum_speed_mps=16.0; shortLeg.segments[1].maximum_speed_mps=16.0;
 assert(VariableSpeedProfileSolver::calculate(shortLeg).status==SpeedProfileStatus::Infeasible);
 auto wrong=in; wrong.speed_reference="GROUND_SPEED"; assert(VariableSpeedProfileSolver::calculate(wrong).status==SpeedProfileStatus::Infeasible);
 return 0;
}
