#include "insurance_component_lifecycle.hpp"

#include <cassert>
#include <iostream>
#include <vector>

using namespace bluesky::insurance;

int main() {
    std::vector<ComponentLifecycleObservation> observations{
        {"OBS-1","UAV-1","PROP-1","propulsion","CFG-1",ComponentLifecycleEventType::Installed,
         ComponentLifecycleStatus::Active,10.0,100,1000,"SRC-1","EVD-1","maintenance-db","1.0",true},
        {"OBS-2","UAV-1","PROP-1","propulsion","CFG-1",ComponentLifecycleEventType::Inspection,
         ComponentLifecycleStatus::Due,14.5,130,1100,"SRC-2","EVD-2","maintenance-db","1.0",true},
        {"OBS-3","UAV-2","PROP-1","propulsion","CFG-1",ComponentLifecycleEventType::Inspection,
         ComponentLifecycleStatus::Active,99.0,999,1200,"SRC-3","EVD-3","maintenance-db","1.0",true}
    };

    InsuranceComponentLifecycleCalculator calc("UAV-1");
    const auto metrics = calc.calculate(observations, 2000);

    assert(metrics.size() == 1);
    const auto& m = metrics.front();
    assert(m.uav_id == "UAV-1");
    assert(m.component_id == "PROP-1");
    assert(m.operating_hours_delta == 4.5);
    assert(m.cycle_delta == 30);
    assert(m.latest_status == ComponentLifecycleStatus::Due);
    assert(m.observation_count == 2);
    assert(m.source_record_ids.size() == 2);
    assert(m.evidence_refs.size() == 2);
    assert(m.evidence_valid);
    assert(m.authoritative);
    assert(m.method_version == "1.0");

    std::cout << "insurance_component_lifecycle_test: PASS\n";
    return 0;
}
