#include "insurance_risk_source_adapter.hpp"
#include <cassert>
#include <iostream>
using namespace bluesky::insurance;
int main(){
 InsuranceRiskSourceAdapter a("UAV-001");
 assert(a.ingest({"BAT-1","UAV-001","BAT-65","FL-1","BATTERY_SOH","1.0",0.91,0.91,35.0,42,1000,true}));
 assert(a.ingest({"PROP-1","UAV-001","M1","FL-1","PROP_FAULT_RATE","1.0",120.0,0.01,55.0,80,1000,true}));
 assert(a.ingest({"NAV-1","UAV-001","FL-1","C2GNSS","1.0",2.0,80.0,1.5,3,1000,true}));
 assert(a.ingest({"ENV-1","UAV-001","FL-1","WEATHER","1.0",2.0,8.0,14.0,12.0,0.4,true,1000,true,"CFG-A"}));
 assert(a.risk().observations().size()==5);
 assert(a.risk().adverse_weather_hours()==2.0);
 assert(!a.ingest({"BAD","UAV-999","BAT-1","FL-1","M","1",0.9,0.9,20,1,1000,true}));
 std::cout<<"insurance_risk_source_adapter_test: PASS\n";
}
