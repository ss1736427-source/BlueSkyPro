#include "insurance_uav_history.hpp"

#include <algorithm>
#include <utility>

namespace bluesky::insurance {

InsuranceUavHistory::InsuranceUavHistory(std::string uav_id)
    : uav_id_(std::move(uav_id)) {}

bool InsuranceUavHistory::add_flight(const FlightExposureRecord& record) {
    if (record.uav_id != uav_id_ || record.flight_record_id.empty() ||
        record.flight_hours < 0.0 || record.evidence != EvidenceIntegrity::Valid) {
        return false;
    }
    const auto duplicate = std::find_if(
        flights_.begin(), flights_.end(),
        [&](const auto& item) { return item.flight_record_id == record.flight_record_id; });
    if (duplicate != flights_.end()) {
        return false;
    }
    flights_.push_back(record);
    flight_hours_ += record.flight_hours;
    ++flight_count_;
    cycles_ += record.cycles;
    return true;
}

bool InsuranceUavHistory::add_event(const HistoryEvent& event) {
    if (event.uav_id != uav_id_ || event.event_id.empty() ||
        event.evidence == EvidenceIntegrity::Missing ||
        event.evidence == EvidenceIntegrity::Corrupt) {
        return false;
    }
    const auto duplicate = std::find_if(
        events_.begin(), events_.end(),
        [&](const auto& item) { return item.event_id == event.event_id; });
    if (duplicate != events_.end()) {
        return false;
    }
    events_.push_back(event);
    return true;
}

bool InsuranceUavHistory::add_configuration_change(const ConfigurationChange& change) {
    if (change.uav_id != uav_id_ || change.change_id.empty() ||
        change.configuration_id.empty() ||
        change.evidence == EvidenceIntegrity::Missing ||
        change.evidence == EvidenceIntegrity::Corrupt) {
        return false;
    }
    const auto duplicate = std::find_if(
        configuration_history_.begin(), configuration_history_.end(),
        [&](const auto& item) { return item.change_id == change.change_id; });
    if (duplicate != configuration_history_.end()) {
        return false;
    }
    configuration_history_.push_back(change);
    return true;
}

bool InsuranceUavHistory::add_derived_metric(const DerivedMetric& metric) {
    if (metric.uav_id != uav_id_ || metric.metric_id.empty() ||
        metric.calculation_method.empty() || metric.method_version.empty() ||
        metric.source_record_ids.empty()) {
        return false;
    }
    if (!metric_authoritative(metric)) {
        return false;
    }
    const auto duplicate = std::find_if(
        derived_metrics_.begin(), derived_metrics_.end(),
        [&](const auto& item) { return item.metric_id == metric.metric_id; });
    if (duplicate != derived_metrics_.end()) {
        return false;
    }
    derived_metrics_.push_back(metric);
    return true;
}

bool InsuranceUavHistory::metric_authoritative(const DerivedMetric& metric) const noexcept {
    if (!metric.authoritative || metric.uav_id != uav_id_ ||
        metric.source_record_ids.empty() || metric.calculation_method.empty() ||
        metric.method_version.empty()) {
        return false;
    }
    for (const auto& source_id : metric.source_record_ids) {
        const auto flight = std::find_if(
            flights_.begin(), flights_.end(),
            [&](const auto& record) { return record.flight_record_id == source_id; });
        if (flight == flights_.end() || flight->evidence != EvidenceIntegrity::Valid) {
            return false;
        }
    }
    return true;
}

}  // namespace bluesky::insurance
