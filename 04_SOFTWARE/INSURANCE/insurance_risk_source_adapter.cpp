#include "insurance_risk_source_adapter.hpp"
#include <utility>
namespace bluesky::insurance {
InsuranceRiskSourceAdapter::InsuranceRiskSourceAdapter(std::string id):uav_id_(std::move(id)),risk_(uav_id_){}
bool InsuranceRiskSourceAdapter::ingest(const BatteryObservation& x){
 if(x.uav_id!=uav_id_||x.id.empty()||x.battery_id.empty()||x.source_record_id.empty()||x.method.empty()||x.method_version.empty()||!x.evidence_valid)return false;
 return risk_.add_observation({x.id,x.uav_id,RiskDimension::Battery,std::to_string(x.state_of_health),"state_of_health",x.timestamp_epoch,x.source_record_id,x.method,x.method_version,x.evidence_valid});
}
bool InsuranceRiskSourceAdapter::ingest(const PropulsionObservation& x){
 if(x.uav_id!=uav_id_||x.id.empty()||x.component_id.empty()||x.source_record_id.empty()||x.method.empty()||x.method_version.empty()||!x.evidence_valid||x.operating_hours<0||x.fault_rate<0)return false;
 return risk_.add_observation({x.id,x.uav_id,RiskDimension::Propulsion,std::to_string(x.fault_rate),"fault_rate",x.timestamp_epoch,x.source_record_id,x.method,x.method_version,x.evidence_valid});
}
bool InsuranceRiskSourceAdapter::ingest(const C2GnssObservation& x){
 if(x.uav_id!=uav_id_||x.id.empty()||x.source_record_id.empty()||x.method.empty()||x.method_version.empty()||!x.evidence_valid||x.c2_loss_seconds<0||x.c2_latency_ms<0||x.gnss_deviation_m<0)return false;
 const auto a=risk_.add_observation({x.id+"-C2",x.uav_id,RiskDimension::C2,std::to_string(x.c2_loss_seconds),"c2_loss_seconds",x.timestamp_epoch,x.source_record_id,x.method,x.method_version,x.evidence_valid});
 if(!a)return false;
 return risk_.add_observation({x.id+"-GNSS",x.uav_id,RiskDimension::NavigationGnss,std::to_string(x.gnss_deviation_m),"gnss_deviation_m",x.timestamp_epoch,x.source_record_id,x.method,x.method_version,x.evidence_valid});
}
bool InsuranceRiskSourceAdapter::ingest(const EnvironmentalObservation& x){
 if(x.uav_id!=uav_id_||x.id.empty()||x.source_record_id.empty()||x.method.empty()||x.method_version.empty()||!x.evidence_valid||x.flight_hours<0||x.wind_mps<0||x.gust_mps<0||x.precipitation_mm<0)return false;
 const auto a=risk_.add_observation({x.id,x.uav_id,RiskDimension::Environmental,std::to_string(x.gust_mps),"gust_mps",x.timestamp_epoch,x.source_record_id,x.method,x.method_version,x.evidence_valid});
 if(!a)return false;
 return risk_.add_exposure({x.id+"-EXP",x.uav_id,x.flight_hours,0,false,0.0,x.adverse?x.flight_hours:0.0,x.configuration_id,x.source_record_id,x.evidence_valid,x.timestamp_epoch});
}
} // namespace bluesky::insurance
