#include "insurance_uav_risk_vector.hpp"
#include <algorithm>
#include <utility>
namespace bluesky::insurance {
InsuranceUavRiskVector::InsuranceUavRiskVector(std::string uav_id): uav_id_(std::move(uav_id)) {}
bool InsuranceUavRiskVector::same_uav(const std::string& id) const noexcept { return !uav_id_.empty() && id == uav_id_; }
bool InsuranceUavRiskVector::add_observation(const RiskObservation& x) {
    if (!same_uav(x.uav_id) || x.observation_id.empty() || x.source_record_id.empty() ||
        x.method.empty() || x.method_version.empty() || !x.evidence_valid) return false;
    if (std::find_if(observations_.begin(), observations_.end(), [&](const auto& v){return v.observation_id==x.observation_id;}) != observations_.end()) return false;
    observations_.push_back(x); return true;
}
bool InsuranceUavRiskVector::add_exposure(const ExposureObservation& x) {
    if (!same_uav(x.uav_id) || x.exposure_id.empty() || x.source_record_id.empty() || !x.evidence_valid ||
        x.flight_hours < 0.0 || x.altitude_hours < 0.0 || x.adverse_weather_hours < 0.0) return false;
    if (std::find_if(exposures_.begin(), exposures_.end(), [&](const auto& v){return v.exposure_id==x.exposure_id;}) != exposures_.end()) return false;
    exposures_.push_back(x); flight_hours_ += x.flight_hours; cycles_ += x.cycles;
    if (x.bvlos) bvlos_hours_ += x.flight_hours; adverse_weather_hours_ += x.adverse_weather_hours; return true;
}
bool InsuranceUavRiskVector::add_claim_evidence(const ClaimsEvidence& x) {
    if (!same_uav(x.uav_id) || x.claim_id.empty() || x.event_id.empty() || x.evidence_refs.empty() || !x.evidence_valid) return false;
    if (std::find_if(claims_evidence_.begin(), claims_evidence_.end(), [&](const auto& v){return v.claim_id==x.claim_id;}) != claims_evidence_.end()) return false;
    claims_evidence_.push_back(x); return true;
}
} // namespace bluesky::insurance
