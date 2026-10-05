#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace bluesky::insurance {

enum class ComponentLifecycleEventType {
    Installed,
    Removed,
    Maintenance,
    Inspection,
    Fault,
    ResourceUpdate,
    ConfigurationChange
};

enum class ComponentLifecycleStatus {
    Active,
    Due,
    Restricted,
    Removed,
    Unknown
};

struct ComponentLifecycleObservation {
    std::string observation_id;
    std::string uav_id;
    std::string component_id;
    std::string component_type;
    std::string configuration_id;
    ComponentLifecycleEventType event_type{ComponentLifecycleEventType::Inspection};
    ComponentLifecycleStatus status{ComponentLifecycleStatus::Unknown};
    double operating_hours{0.0};
    std::uint64_t cycles{0};
    std::int64_t timestamp_epoch{0};
    std::string source_record_id;
    std::string evidence_ref;
    std::string method;
    std::string method_version;
    bool evidence_valid{false};
};

struct ComponentLifecycleMetric {
    std::string metric_id;
    std::string uav_id;
    std::string component_id;
    std::string component_type;
    std::string configuration_id;
    double operating_hours_delta{0.0};
    std::uint64_t cycle_delta{0};
    std::uint64_t observation_count{0};
    ComponentLifecycleStatus latest_status{ComponentLifecycleStatus::Unknown};
    std::int64_t first_observation_epoch{0};
    std::int64_t last_observation_epoch{0};
    std::vector<std::string> source_record_ids;
    std::vector<std::string> evidence_refs;
    std::string calculation_method;
    std::string method_version;
    std::int64_t calculated_at_epoch{0};
    bool evidence_valid{false};
    bool authoritative{false};
};

class InsuranceComponentLifecycleCalculator {
public:
    explicit InsuranceComponentLifecycleCalculator(std::string uav_id);

    std::vector<ComponentLifecycleMetric> calculate(
        const std::vector<ComponentLifecycleObservation>& observations,
        std::int64_t calculated_at_epoch) const;

private:
    std::string uav_id_;
};

}  // namespace bluesky::insurance
