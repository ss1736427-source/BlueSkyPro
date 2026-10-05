#pragma once
#include <cstdint>
#include <string>
#include <vector>
namespace bluesky::insurance {
enum class RiskDimension { Technical, Maintenance, Battery, Propulsion, C2, NavigationGnss, Operational, Environmental, Compliance, PilotOperator, EvidenceQuality };
struct RiskObservation {
    std::string observation_id; std::string uav_id; RiskDimension dimension{RiskDimension::Technical};
    std::string value; std::string unit; std::int64_t timestamp_epoch{0}; std::string source_record_id;
    std::string method; std::string method_version; bool evidence_valid{false};
};
struct ExposureObservation {
    std::string exposure_id; std::string uav_id; double flight_hours{0.0}; std::uint64_t cycles{0};
    bool bvlos{false}; double altitude_hours{0.0}; double adverse_weather_hours{0.0};
    std::string configuration_id; std::string source_record_id; bool evidence_valid{false}; std::int64_t timestamp_epoch{0};
};
struct ClaimsEvidence {
    std::string claim_id; std::string uav_id; std::string event_id; std::vector<std::string> evidence_refs;
    std::int64_t created_at_epoch{0}; bool evidence_valid{false};
};
class InsuranceUavRiskVector {
public:
    explicit InsuranceUavRiskVector(std::string uav_id);
    const std::string& uav_id() const noexcept { return uav_id_; }
    bool add_observation(const RiskObservation&);
    bool add_exposure(const ExposureObservation&);
    bool add_claim_evidence(const ClaimsEvidence&);
    const std::vector<RiskObservation>& observations() const noexcept { return observations_; }
    const std::vector<ExposureObservation>& exposures() const noexcept { return exposures_; }
    const std::vector<ClaimsEvidence>& claims_evidence() const noexcept { return claims_evidence_; }
    double flight_hours() const noexcept { return flight_hours_; }
    std::uint64_t cycles() const noexcept { return cycles_; }
    double bvlos_hours() const noexcept { return bvlos_hours_; }
    double adverse_weather_hours() const noexcept { return adverse_weather_hours_; }
private:
    bool same_uav(const std::string&) const noexcept;
    std::string uav_id_; double flight_hours_{0.0}; std::uint64_t cycles_{0};
    double bvlos_hours_{0.0}; double adverse_weather_hours_{0.0};
    std::vector<RiskObservation> observations_; std::vector<ExposureObservation> exposures_;
    std::vector<ClaimsEvidence> claims_evidence_;
};
} // namespace bluesky::insurance
