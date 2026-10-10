#pragma once
#include "insurance_uav_risk_vector.hpp"
#include <cstdint>
#include <string>
#include <vector>

namespace bluesky::insurance {

struct RiskTimeWindow {
    std::int64_t start_epoch{0};
    std::int64_t end_epoch{0};
};

enum class TemporalMetricKind {
    ObservationMeanDelta,
    ObservationRateOfChange,
    ConfigurationExposureDelta,
    ConfigurationBvlosRatio,
    ConfigurationAdverseWeatherRatio,
    ConfigurationAltitudeRatio
};

struct TemporalRiskMetric {
    std::string metric_id;
    std::string uav_id;
    std::string configuration_id;
    RiskDimension dimension{RiskDimension::Technical};
    TemporalMetricKind kind{TemporalMetricKind::ObservationMeanDelta};
    double value{0.0};
    std::string unit;
    RiskTimeWindow baseline;
    RiskTimeWindow current;
    std::uint64_t baseline_sample_count{0};
    std::uint64_t current_sample_count{0};
    double baseline_value{0.0};
    double current_value{0.0};
    double exposure_denominator{0.0};
    std::string calculation_method;
    std::string method_version;
    std::int64_t calculated_at_epoch{0};
    std::vector<std::string> source_record_ids;
    bool evidence_valid{false};
    bool authoritative{false};
};

class InsuranceRiskTemporalCalculator {
public:
    explicit InsuranceRiskTemporalCalculator(std::string uav_id);
    std::vector<TemporalRiskMetric> calculate(
        const InsuranceUavRiskVector& risk,
        RiskTimeWindow baseline,
        RiskTimeWindow current,
        std::int64_t calculated_at_epoch) const;

private:
    std::string uav_id_;
};

} // namespace bluesky::insurance
