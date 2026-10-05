#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace bluesky::insurance {

enum class BatteryLifecycleMetricKind {
    StateOfHealthDelta,
    StateOfHealthLossPerCycle,
    CycleDelta,
    TemperatureMean,
    ObservationAgeSeconds
};

struct BatteryLifecycleObservation {
    std::string observation_id;
    std::string uav_id;
    std::string battery_id;
    std::string source_record_id;
    double state_of_health{0.0};
    double temperature_c{0.0};
    std::uint64_t cycles{0};
    std::int64_t timestamp_epoch{0};
    std::string configuration_id;
    std::string method;
    std::string method_version;
    bool evidence_valid{false};
};

struct BatteryLifecycleMetric {
    std::string metric_id;
    std::string uav_id;
    std::string battery_id;
    std::string configuration_id;
    BatteryLifecycleMetricKind kind{BatteryLifecycleMetricKind::StateOfHealthDelta};
    double value{0.0};
    std::string unit;
    std::int64_t first_observation_epoch{0};
    std::int64_t last_observation_epoch{0};
    std::uint64_t sample_count{0};
    std::vector<std::string> source_record_ids;
    std::string calculation_method;
    std::string method_version;
    std::int64_t calculated_at_epoch{0};
    bool evidence_valid{false};
    bool authoritative{false};
};

class InsuranceBatteryLifecycleCalculator {
public:
    explicit InsuranceBatteryLifecycleCalculator(std::string uav_id);
    std::vector<BatteryLifecycleMetric> calculate(
        const std::vector<BatteryLifecycleObservation>& observations,
        std::int64_t calculated_at_epoch) const;
private:
    std::string uav_id_;
};

} // namespace bluesky::insurance
