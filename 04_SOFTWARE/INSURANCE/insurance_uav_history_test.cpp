#include "insurance_uav_history.hpp"

#include <cassert>
#include <iostream>

using namespace bluesky::insurance;

int main() {
    InsuranceUavHistory history("UAV-001");

    assert(history.add_flight({"FLIGHT-001", "UAV-001", "CFG-A", 1.5, 2, 1000,
                               EvidenceIntegrity::Valid}));
    assert(history.flight_count() == 1);
    assert(history.flight_hours() == 1.5);
    assert(history.cycles() == 2);

    assert(history.add_configuration_change(
        {"CFG-CHANGE-001", "UAV-001", "CFG-B", "CFG-A", 1100,
         "EVID-CONFIG-001", EvidenceIntegrity::Valid}));
    assert(history.configuration_history().size() == 1);

    assert(history.add_flight({"FLIGHT-002", "UAV-001", "CFG-B", 2.0, 3, 1200,
                               EvidenceIntegrity::Valid}));
    assert(history.flight_count() == 2);
    assert(history.flight_hours() == 3.5);
    assert(history.cycles() == 5);

    assert(history.add_event({"EVENT-001", "UAV-001", InsuranceHistoryEventType::Incident,
                              "CFG-B", 1250, "EVID-INC-001", EvidenceIntegrity::Valid}));
    assert(history.events().size() == 1);

    DerivedMetric metric;
    metric.metric_id = "METRIC-HOURS";
    metric.uav_id = "UAV-001";
    metric.value = 3.5;
    metric.unit = "flight_hours";
    metric.calculation_method = "SUM_VALID_FLIGHT_EXPOSURE";
    metric.method_version = "1.0";
    metric.calculated_at_epoch = 1300;
    metric.source_record_ids = {"FLIGHT-001", "FLIGHT-002"};
    metric.authoritative = true;
    assert(history.add_derived_metric(metric));
    assert(history.derived_metrics().size() == 1);
    assert(history.metric_authoritative(metric));

    assert(!history.add_flight({"FLIGHT-003", "UAV-999", "CFG-B", 1.0, 1, 1400,
                                EvidenceIntegrity::Valid}));
    assert(!history.add_flight({"FLIGHT-004", "UAV-001", "CFG-B", 1.0, 1, 1400,
                                EvidenceIntegrity::Missing}));

    DerivedMetric missing_source = metric;
    missing_source.metric_id = "METRIC-BAD";
    missing_source.source_record_ids = {"FLIGHT-001", "MISSING"};
    assert(!history.add_derived_metric(missing_source));
    assert(!history.metric_authoritative(missing_source));

    DerivedMetric no_lineage = metric;
    no_lineage.metric_id = "METRIC-NOLINEAGE";
    no_lineage.source_record_ids.clear();
    assert(!history.add_derived_metric(no_lineage));

    std::cout << "insurance_uav_history_test: PASS\n";
}
