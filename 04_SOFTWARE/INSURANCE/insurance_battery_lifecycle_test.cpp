#include "insurance_battery_lifecycle.hpp"
#include <cassert>
#include <cmath>
#include <iostream>
using namespace bluesky::insurance;
int main(){
 InsuranceBatteryLifecycleCalculator c("UAV-1");
 std::vector<BatteryLifecycleObservation> v={
 {"B1-1","UAV-1","BAT-A","SRC-1",0.95,25.0,10,100,"CFG-A","soh","1.0",true},
 {"B1-2","UAV-1","BAT-A","SRC-2",0.90,30.0,20,200,"CFG-A","soh","1.0",true},
 {"B1-3","UAV-1","BAT-A","SRC-3",0.85,35.0,30,300,"CFG-A","soh","1.0",true}};
 auto m=c.calculate(v,400);
 assert(m.size()==5);
 bool d=false,l=false,cy=false,t=false,age=false;
 for(const auto& x:m){
  assert(x.uav_id=="UAV-1"&&x.battery_id=="BAT-A"&&x.configuration_id=="CFG-A");
  assert(x.authoritative&&x.evidence_valid&&x.method_version=="1.0");
  if(x.kind==BatteryLifecycleMetricKind::StateOfHealthDelta){d=true;assert(std::abs(x.value+0.10)<1e-9);}
  if(x.kind==BatteryLifecycleMetricKind::StateOfHealthLossPerCycle){l=true;assert(std::abs(x.value-0.005)<1e-9);}
  if(x.kind==BatteryLifecycleMetricKind::CycleDelta){cy=true;assert(std::abs(x.value-20.0)<1e-9);}
  if(x.kind==BatteryLifecycleMetricKind::TemperatureMean){t=true;assert(std::abs(x.value-30.0)<1e-9);}
  if(x.kind==BatteryLifecycleMetricKind::ObservationAgeSeconds){age=true;assert(std::abs(x.value-200.0)<1e-9);}
  assert(x.source_record_ids.size()==3);
 }
 assert(d&&l&&cy&&t&&age);
 auto bad=v; bad[1].evidence_valid=false;
 assert(c.calculate(bad,400).size()==5);
 std::cout<<"insurance_battery_lifecycle_test: PASS\n";
}
