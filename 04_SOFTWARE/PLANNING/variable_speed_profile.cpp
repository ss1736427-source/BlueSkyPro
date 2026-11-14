#include "variable_speed_profile.hpp"
#include <algorithm>
#include <cmath>
namespace bluesky::planning {
namespace { bool positiveFinite(double v) { return std::isfinite(v) && v > 0.0; } }
SpeedProfileResult VariableSpeedProfileSolver::calculate(const SpeedProfileInput& input) {
 SpeedProfileResult out;
 out.vehicle_model_id=input.vehicle_model_id; out.vehicle_model_version=input.vehicle_model_version;
 out.configuration_id=input.configuration_id; out.speed_reference=input.speed_reference;
 out.dynamics_source=input.longitudinal_dynamics_source;
 const auto fail=[&](const std::string& why){ out.status=SpeedProfileStatus::Infeasible; out.segments.clear(); out.findings.push_back(why); return out; };
 if(input.vehicle_model_id.empty()||input.vehicle_model_version.empty()||input.configuration_id.empty()||
    (input.speed_reference!="TAS"&&input.speed_reference!="AIR_RELATIVE"))
  return fail("SPEED_PROFILE_VEHICLE_BINDING_INVALID: exact vehicle/configuration and explicit air-relative speed reference are required.");
 if(!input.longitudinal_dynamics_validated||input.longitudinal_dynamics_source.empty()||
    !positiveFinite(input.maximum_acceleration_mps2)||!positiveFinite(input.maximum_deceleration_mps2))
  return fail("SPEED_PROFILE_LONGITUDINAL_EVIDENCE_MISSING: validated acceleration/deceleration limits and provenance are mandatory.");
 if(input.segments.empty()||!positiveFinite(input.initial_speed_mps)||!positiveFinite(input.final_speed_mps))
  return fail("SPEED_PROFILE_ROUTE_INVALID: non-empty segments and positive boundary speeds are required.");
 const std::size_t n=input.segments.size();
 std::vector<double> caps(n+1), mins(n+1);
 for(std::size_t i=0;i<n;++i) {
  const auto& s=input.segments[i];
  if(s.segment_id.empty()||!positiveFinite(s.distance_m)||!positiveFinite(s.minimum_speed_mps)||
     !positiveFinite(s.maximum_speed_mps)||s.minimum_speed_mps>s.maximum_speed_mps)
   return fail("SPEED_PROFILE_SEGMENT_INVALID: every segment needs identity, positive length and a valid speed envelope.");
  if(i&&s.segment_id==input.segments[i-1].segment_id)
   return fail("SPEED_PROFILE_SEGMENT_ID_DUPLICATE: adjacent segment identities must differ.");
  if(i==0){caps[0]=s.maximum_speed_mps;mins[0]=s.minimum_speed_mps;}
  else {caps[i]=std::min(input.segments[i-1].maximum_speed_mps,s.maximum_speed_mps);mins[i]=std::max(input.segments[i-1].minimum_speed_mps,s.minimum_speed_mps);}
  caps[i+1]=s.maximum_speed_mps; mins[i+1]=s.minimum_speed_mps;
 }
 if(input.initial_speed_mps<mins.front()||input.initial_speed_mps>caps.front()||
    input.final_speed_mps<mins.back()||input.final_speed_mps>caps.back())
  return fail("SPEED_PROFILE_BOUNDARY_SPEED_INVALID: initial/final speed conflicts with adjacent segment envelope.");
 std::vector<double> v=caps; v.front()=input.initial_speed_mps; v.back()=input.final_speed_mps;
 // Backward braking envelope.
 for(std::size_t i=n;i-->0;) v[i]=std::min(v[i],std::sqrt(v[i+1]*v[i+1]+2.0*input.maximum_deceleration_mps2*input.segments[i].distance_m));
 if(v.front()+1e-9<input.initial_speed_mps)
  return fail("SPEED_PROFILE_INITIAL_BRAKING_INFEASIBLE: initial speed cannot reach downstream envelope.");
 // Forward acceleration envelope.
 v.front()=input.initial_speed_mps;
 for(std::size_t i=0;i<n;++i) v[i+1]=std::min(v[i+1],std::sqrt(v[i]*v[i]+2.0*input.maximum_acceleration_mps2*input.segments[i].distance_m));
 if(v.back()+1e-9<input.final_speed_mps)
  return fail("SPEED_PROFILE_FINAL_ACCELERATION_INFEASIBLE: final boundary speed is unreachable.");
 for(std::size_t i=0;i<=n;++i) if(v[i]+1e-9<mins[i])
  return fail("SPEED_PROFILE_MINIMUM_SPEED_CONFLICT: acceleration/braking envelope falls below an adjacent segment minimum speed.");
 out.segments.reserve(n);
 for(std::size_t i=0;i<n;++i) {
  const auto& s=input.segments[i]; const double entry=v[i], exit=v[i+1];
  const double peak2=(2.0*s.distance_m+entry*entry/input.maximum_acceleration_mps2+exit*exit/input.maximum_deceleration_mps2)/
    (1.0/input.maximum_acceleration_mps2+1.0/input.maximum_deceleration_mps2);
  const double peak=std::min(s.maximum_speed_mps,std::sqrt(std::max(0.0,peak2)));
  if(peak+1e-9<std::max(entry,exit)) return fail("SPEED_PROFILE_TRANSITION_INFEASIBLE: segment cannot connect its boundary speeds within its length.");
  const double ad=std::max(0.0,(peak*peak-entry*entry)/(2.0*input.maximum_acceleration_mps2));
  const double dd=std::max(0.0,(peak*peak-exit*exit)/(2.0*input.maximum_deceleration_mps2));
  const double cd=s.distance_m-ad-dd;
  if(cd < -1e-7) return fail("SPEED_PROFILE_DISTANCE_OVERCOMMITTED: acceleration and braking exceed segment length.");
  out.segments.push_back({s.segment_id,entry,exit,peak,ad,dd,std::max(0.0,cd)});
 }
 out.status=SpeedProfileStatus::Feasible; return out;
}
} // namespace bluesky::planning
