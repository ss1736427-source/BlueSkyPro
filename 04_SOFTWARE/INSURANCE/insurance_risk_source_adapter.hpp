#pragma once
#include "insurance_uav_risk_vector.hpp"
#include <string>
namespace bluesky::insurance {
struct BatteryObservation {
 std::string id,uav_id,battery_id,source_record_id,method,method_version;
 double state_of_health{0.0},remaining_capacity_ratio{0.0},temperature_c{0.0}; std::uint64_t cycles{0};
 std::int64_t timestamp_epoch{0}; bool evidence_valid{false};
};
struct PropulsionObservation {
 std::string id,uav_id,component_id,source_record_id,method,method_version;
 double operating_hours{0.0},fault_rate{0.0},temperature_c{0.0}; std::uint64_t cycles{0};
 std::int64_t timestamp_epoch{0}; bool evidence_valid{false};
};
struct C2GnssObservation {
 std::string id,uav_id,source_record_id,method,method_version;
 double c2_loss_seconds{0.0},c2_latency_ms{0.0},gnss_deviation_m{0.0}; std::uint64_t gnss_events{0};
 std::int64_t timestamp_epoch{0}; bool evidence_valid{false};
};
struct EnvironmentalObservation {
 std::string id,uav_id,source_record_id,method,method_version;
 double flight_hours{0.0},wind_mps{0.0},gust_mps{0.0},temperature_c{0.0},precipitation_mm{0.0}; bool adverse{false};
 std::int64_t timestamp_epoch{0}; bool evidence_valid{false}; std::string configuration_id;
};
class InsuranceRiskSourceAdapter {
public:
 explicit InsuranceRiskSourceAdapter(std::string uav_id);
 bool ingest(const BatteryObservation&);
 bool ingest(const PropulsionObservation&);
 bool ingest(const C2GnssObservation&);
 bool ingest(const EnvironmentalObservation&);
 const InsuranceUavRiskVector& risk() const noexcept { return risk_; }
private:
 std::string uav_id_; InsuranceUavRiskVector risk_;
};
} // namespace bluesky::insurance
