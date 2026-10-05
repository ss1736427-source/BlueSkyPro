#pragma once
#include "insurance_uav_risk_vector.hpp"
#include <cstdint>
#include <string>
#include <vector>

namespace bluesky::insurance {

enum class RiskMetricKind { Mean, Maximum, BvlosExposureRatio, AdverseWeatherExposureRatio, AltitudeExposureRatio, CyclesPerFlightHour };

struct RiskDerivedMetric {
    std::string metric_id;
    std::string uav_id;
    RiskDimension dimension{RiskDimension::Technical};
    RiskMetricKind kind{RiskMetricKind::Mean};
    double value{0.0};
    std::string unit;
    std::uint64_t sample_count{0};
    double exposure_denominator{0.0};
    std::string calculation_method;
    std::string method_version;
    std::int64_t calculated_at_epoch{0};
    std::vector<std::string> source_record_ids;
    bool evidence_valid{false};
    bool authoritative{false};
};

class InsuranceRiskCalculator {
public:
    explicit InsuranceRiskCalculator(std::string uav_id);
    std::vector<RiskDerivedMetric> calculate(const InsuranceUavRiskVector& risk,
                                             std::int64_t calculated_at_epoch) const;

private:
    std::string uav_id_;
};

} // namespace bluesky::insurance
