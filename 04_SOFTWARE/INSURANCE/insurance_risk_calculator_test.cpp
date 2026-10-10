#include "insurance_risk_calculator.hpp"
#include <cassert>
#include <cmath>
#include <iostream>

using namespace bluesky::insurance;

int main() {
    InsuranceUavRiskVector risk("UAV-1");
    assert(risk.add_observation({"BAT-1", "UAV-1", RiskDimension::Battery, "0.8", "ratio", 100, "BAT-SRC-1", "battery-health", "1.0", true}));
    assert(risk.add_observation({"BAT-2", "UAV-1", RiskDimension::Battery, "0.6", "ratio", 101, "BAT-SRC-2", "battery-health", "1.0", true}));
    assert(risk.add_observation({"ENV-1", "UAV-1", RiskDimension::Environmental, "12.0", "m/s", 102, "ENV-SRC-1", "wind-gust", "1.0", true}));
    assert(risk.add_exposure({"EXP-1", "UAV-1", 2.0, 3, true, 1.0, 1.0, "CFG-A", "EXP-SRC-1", true}));
    assert(risk.add_exposure({"EXP-2", "UAV-1", 3.0, 2, false, 2.0, 0.0, "CFG-A", "EXP-SRC-2", true}));

    InsuranceRiskCalculator calculator("UAV-1");
    const auto metrics = calculator.calculate(risk, 200);
    assert(metrics.size() == 10);

    bool battery_mean = false;
    bool battery_max = false;
    bool bvlos_ratio = false;
    bool adverse_ratio = false;
    bool altitude_ratio = false;
    bool cycles_rate = false;

    for (const auto& m : metrics) {
        assert(m.uav_id == "UAV-1");
        assert(m.authoritative);
        assert(m.evidence_valid);
        assert(!m.source_record_ids.empty());
        assert(m.method_version == "1.0");
        assert(m.calculated_at_epoch == 200);
        if (m.kind == RiskMetricKind::Mean && m.dimension == RiskDimension::Battery) {
            battery_mean = true; assert(std::abs(m.value - 0.7) < 1e-9);
        }
        if (m.kind == RiskMetricKind::Maximum && m.dimension == RiskDimension::Battery) {
            battery_max = true; assert(std::abs(m.value - 0.8) < 1e-9);
        }
        if (m.kind == RiskMetricKind::BvlosExposureRatio) {
            bvlos_ratio = true; assert(std::abs(m.value - 0.4) < 1e-9);
        }
        if (m.kind == RiskMetricKind::AdverseWeatherExposureRatio) {
            adverse_ratio = true; assert(std::abs(m.value - 0.2) < 1e-9);
        }
        if (m.kind == RiskMetricKind::AltitudeExposureRatio) {
            altitude_ratio = true; assert(std::abs(m.value - 0.6) < 1e-9);
        }
        if (m.kind == RiskMetricKind::CyclesPerFlightHour) {
            cycles_rate = true; assert(std::abs(m.value - 1.0) < 1e-9);
        }
    }

    assert(battery_mean && battery_max && bvlos_ratio && adverse_ratio && altitude_ratio && cycles_rate);

    InsuranceUavRiskVector wrong("UAV-2");
    assert(calculator.calculate(wrong, 200).empty());

    std::cout << "insurance_risk_calculator_test: PASS\\n";
    return 0;
}
