#include "insurance_risk_temporal.hpp"
#include <cassert>
#include <cmath>
#include <iostream>

using namespace bluesky::insurance;

int main() {
    InsuranceUavRiskVector risk("UAV-1");
    assert(risk.add_observation({"OBS-1","UAV-1",RiskDimension::Battery,"0.8","ratio",10,"SRC-1","battery","1.0",true}));
    assert(risk.add_observation({"OBS-2","UAV-1",RiskDimension::Battery,"0.6","ratio",20,"SRC-2","battery","1.0",true}));
    assert(risk.add_observation({"OBS-3","UAV-1",RiskDimension::Battery,"0.5","ratio",110,"SRC-3","battery","1.0",true}));
    assert(risk.add_observation({"OBS-4","UAV-1",RiskDimension::Battery,"0.4","ratio",120,"SRC-4","battery","1.0",true}));

    assert(risk.add_exposure({"EXP-1","UAV-1",2.0,2,true,1.0,0.0,"CFG-A","E1",true,10}));
    assert(risk.add_exposure({"EXP-2","UAV-1",2.0,2,false,0.0,0.5,"CFG-A","E2",true,20}));
    assert(risk.add_exposure({"EXP-3","UAV-1",4.0,4,true,2.0,2.0,"CFG-A","E3",true,110}));
    assert(risk.add_exposure({"EXP-4","UAV-1",4.0,4,false,1.0,0.0,"CFG-A","E4",true,120}));
    assert(risk.add_exposure({"EXP-5","UAV-1",1.0,1,false,0.5,0.0,"CFG-B","E5",true,120}));

    InsuranceRiskTemporalCalculator calculator("UAV-1");
    const auto metrics = calculator.calculate(risk,{0,100},{100,200},200);
    assert(metrics.size() == 6);

    bool delta=false, rate=false, hours=false, bvlos=false, adverse=false, altitude=false;
    for (const auto& m : metrics) {
        assert(m.uav_id=="UAV-1");
        assert(m.evidence_valid && m.authoritative);
        assert(m.method_version=="1.0");
        assert(!m.source_record_ids.empty());
        assert(m.calculated_at_epoch==200);
        if (m.kind==TemporalMetricKind::ObservationMeanDelta) {
            delta=true; assert(std::abs(m.value - (-0.25)) < 1e-9);
        } else if (m.kind==TemporalMetricKind::ObservationRateOfChange) {
            rate=true; assert(std::abs(m.value - (-0.0025)) < 1e-9);
        } else if (m.kind==TemporalMetricKind::ConfigurationExposureDelta && m.configuration_id=="CFG-A") {
            hours=true; assert(std::abs(m.value-4.0)<1e-9);
        } else if (m.kind==TemporalMetricKind::ConfigurationBvlosRatio && m.configuration_id=="CFG-A") {
            bvlos=true; assert(std::abs(m.value-0.5)<1e-9);
        } else if (m.kind==TemporalMetricKind::ConfigurationAdverseWeatherRatio && m.configuration_id=="CFG-A") {
            adverse=true; assert(std::abs(m.value-0.25)<1e-9);
        } else if (m.kind==TemporalMetricKind::ConfigurationAltitudeRatio && m.configuration_id=="CFG-A") {
            altitude=true; assert(std::abs(m.value-0.375)<1e-9);
        }
    }
    assert(delta && rate && hours && bvlos && adverse && altitude);

    InsuranceUavRiskVector wrong("UAV-2");
    assert(calculator.calculate(wrong,{0,100},{100,200},200).empty());
    assert(calculator.calculate(risk,{100,100},{100,200},200).empty());
    assert(calculator.calculate(risk,{0,150},{100,200},200).empty());

    std::cout << "insurance_risk_temporal_test: PASS\n";
    return 0;
}
