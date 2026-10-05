#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace bluesky::insurance {

enum class InsuranceHistoryEventType {
    NormalFlight,
    Maintenance,
    ConfigurationChange,
    Advisory,
    Anomaly,
    Deviation,
    Incident,
    SeriousIncident,
    Accident
};

enum class EvidenceIntegrity {
    Valid,
    Missing,
    Corrupt
};

struct FlightExposureRecord {
    std::string flight_record_id;
    std::string uav_id;
    std::string configuration_id;
    double flight_hours{0.0};
    std::uint64_t cycles{0};
    std::int64_t timestamp_epoch{0};
    EvidenceIntegrity evidence{EvidenceIntegrity::Valid};
};

struct HistoryEvent {
    std::string event_id;
    std::string uav_id;
    InsuranceHistoryEventType type{InsuranceHistoryEventType::NormalFlight};
    std::string configuration_id;
    std::int64_t timestamp_epoch{0};
    std::string evidence_ref;
    EvidenceIntegrity evidence{EvidenceIntegrity::Valid};
};

struct ConfigurationChange {
    std::string change_id;
    std::string uav_id;
    std::string configuration_id;
    std::string previous_configuration_id;
    std::int64_t effective_from_epoch{0};
    std::string evidence_ref;
    EvidenceIntegrity evidence{EvidenceIntegrity::Valid};
};

struct DerivedMetric {
    std::string metric_id;
    std::string uav_id;
    double value{0.0};
    std::string unit;
    std::string calculation_method;
    std::string method_version;
    std::int64_t calculated_at_epoch{0};
    std::vector<std::string> source_record_ids;
    bool authoritative{false};
};

class InsuranceUavHistory {
public:
    explicit InsuranceUavHistory(std::string uav_id);

    const std::string& uav_id() const noexcept { return uav_id_; }
    double flight_hours() const noexcept { return flight_hours_; }
    std::uint64_t flight_count() const noexcept { return flight_count_; }
    std::uint64_t cycles() const noexcept { return cycles_; }

    bool add_flight(const FlightExposureRecord& record);
    bool add_event(const HistoryEvent& event);
    bool add_configuration_change(const ConfigurationChange& change);
    bool add_derived_metric(const DerivedMetric& metric);

    const std::vector<FlightExposureRecord>& flights() const noexcept { return flights_; }
    const std::vector<HistoryEvent>& events() const noexcept { return events_; }
    const std::vector<ConfigurationChange>& configuration_history() const noexcept { return configuration_history_; }
    const std::vector<DerivedMetric>& derived_metrics() const noexcept { return derived_metrics_; }

    bool metric_authoritative(const DerivedMetric& metric) const noexcept;

private:
    std::string uav_id_;
    double flight_hours_{0.0};
    std::uint64_t flight_count_{0};
    std::uint64_t cycles_{0};
    std::vector<FlightExposureRecord> flights_;
    std::vector<HistoryEvent> events_;
    std::vector<ConfigurationChange> configuration_history_;
    std::vector<DerivedMetric> derived_metrics_;
};

}  // namespace bluesky::insurance
