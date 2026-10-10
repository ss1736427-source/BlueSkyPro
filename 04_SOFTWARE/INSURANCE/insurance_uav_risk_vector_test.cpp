#include "insurance_uav_risk_vector.hpp"
#include <cassert>
#include <iostream>
using namespace bluesky::insurance;
int main() {
    InsuranceUavRiskVector risk("UAV-001");
    assert(risk.add_observation({"OBS-BAT-001","UAV-001",RiskDimension::Battery,"0.92","capacity_ratio",1000,"FLIGHT-001","BATTERY_CAPACITY_ESTIMATE","1.0",true}));
    assert(risk.add_observation({"OBS-GNSS-001","UAV-001",RiskDimension::NavigationGnss,"2","navigation_deviation_events",1100,"EVENT-001","COUNT_GNSS_DEVIATIONS","1.0",true}));
    assert(risk.add_exposure({"EXP-001","UAV-001",2.0,3,true,2.0,0.5,"CFG-B","FLIGHT-001",true}));
    assert(risk.flight_hours()==2.0 && risk.cycles()==3 && risk.bvlos_hours()==2.0 && risk.adverse_weather_hours()==0.5);
    assert(risk.add_claim_evidence({"CLAIM-001","UAV-001","EVENT-001",{"EVID-001","LOG-001"},1200,true}));
    assert(risk.claims_evidence().size()==1);
    assert(!risk.add_observation({"OBS-BAD-UAV","UAV-999",RiskDimension::Technical,"x","unit",1300,"FLIGHT-001","METHOD","1.0",true}));
    assert(!risk.add_exposure({"EXP-BAD-EVID","UAV-001",1.0,1,false,0.0,0.0,"CFG-B","FLIGHT-002",false}));
    assert(!risk.add_observation({"OBS-NO-LINEAGE","UAV-001",RiskDimension::Technical,"x","unit",1300,"","METHOD","1.0",true}));
    assert(!risk.add_claim_evidence({"CLAIM-BAD","UAV-001","EVENT-001",{},1300,true}));
    std::cout << "insurance_uav_risk_vector_test: PASS\n";
}
