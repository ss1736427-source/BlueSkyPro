#include "insurance_battery_lifecycle.hpp"
#include <algorithm>
#include <cmath>
#include <map>
#include <utility>

namespace bluesky::insurance {
namespace {
struct Group {
    std::string battery_id;
    std::string configuration_id;
    std::vector<BatteryLifecycleObservation> values;
};
bool valid(const BatteryLifecycleObservation& o) {
    return !o.observation_id.empty() && !o.uav_id.empty() && !o.battery_id.empty() &&
           !o.source_record_id.empty() && o.state_of_health >= 0.0 && o.state_of_health <= 1.0 &&
           std::isfinite(o.state_of_health) && std::isfinite(o.temperature_c) &&
           !o.method.empty() && !o.method_version.empty() && o.evidence_valid;
}
std::string id(const std::string& uav, const std::string& battery, const std::string& suffix) {
    return uav + "-BAT-" + battery + "-" + suffix;
}
BatteryLifecycleMetric make_metric(const std::string& idv, const Group& g,
    BatteryLifecycleMetricKind kind, double value, const std::string& unit,
    std::int64_t at, const std::string& method) {
    BatteryLifecycleMetric m;
    m.metric_id=idv; m.uav_id=g.values.front().uav_id; m.battery_id=g.battery_id;
    m.configuration_id=g.configuration_id; m.kind=kind; m.value=value; m.unit=unit;
    m.first_observation_epoch=g.values.front().timestamp_epoch;
    m.last_observation_epoch=g.values.back().timestamp_epoch;
    m.sample_count=g.values.size(); m.calculation_method=method; m.method_version="1.0";
    m.calculated_at_epoch=at; m.evidence_valid=true; m.authoritative=true;
    for(const auto& v:g.values) m.source_record_ids.push_back(v.source_record_id);
    return m;
}
}
InsuranceBatteryLifecycleCalculator::InsuranceBatteryLifecycleCalculator(std::string uav_id):uav_id_(std::move(uav_id)){}
std::vector<BatteryLifecycleMetric> InsuranceBatteryLifecycleCalculator::calculate(
 const std::vector<BatteryLifecycleObservation>& observations,std::int64_t at) const {
 std::vector<BatteryLifecycleMetric> out;
 if(uav_id_.empty()) return out;
 std::map<std::string,Group> groups;
 for(const auto& o:observations) {
   if(o.uav_id!=uav_id_ || !valid(o)) continue;
   auto key=o.battery_id+"|"+o.configuration_id;
   auto& g=groups[key]; g.battery_id=o.battery_id; g.configuration_id=o.configuration_id; g.values.push_back(o);
 }
 for(auto& [key,g]:groups) {
   std::sort(g.values.begin(),g.values.end(),[](const auto&a,const auto&b){return a.timestamp_epoch<b.timestamp_epoch;});
   if(g.values.size()<2) continue;
   const auto& first=g.values.front(); const auto& last=g.values.back();
   const double soh_delta=last.state_of_health-first.state_of_health;
   const auto cycle_delta = last.cycles>=first.cycles ? last.cycles-first.cycles : 0;
   double temp_sum=0.0; for(const auto& v:g.values) temp_sum+=v.temperature_c;
   out.push_back(make_metric(id(uav_id_,g.battery_id,"SOH-DELTA"),g,BatteryLifecycleMetricKind::StateOfHealthDelta,soh_delta,"ratio",at,"Last valid SOH minus first valid SOH"));
   out.push_back(make_metric(id(uav_id_,g.battery_id,"CYCLE-DELTA"),g,BatteryLifecycleMetricKind::CycleDelta,static_cast<double>(cycle_delta),"cycles",at,"Last cycle count minus first valid cycle count"));
   if(cycle_delta>0) out.push_back(make_metric(id(uav_id_,g.battery_id,"SOH-LOSS-PER-CYCLE"),g,BatteryLifecycleMetricKind::StateOfHealthLossPerCycle,-soh_delta/static_cast<double>(cycle_delta),"ratio/cycle",at,"Negative SOH delta divided by cycle delta"));
   out.push_back(make_metric(id(uav_id_,g.battery_id,"TEMP-MEAN"),g,BatteryLifecycleMetricKind::TemperatureMean,temp_sum/static_cast<double>(g.values.size()),"degC",at,"Arithmetic mean of valid battery temperatures"));
   out.push_back(make_metric(id(uav_id_,g.battery_id,"AGE"),g,BatteryLifecycleMetricKind::ObservationAgeSeconds,static_cast<double>(last.timestamp_epoch-first.timestamp_epoch),"seconds",at,"Elapsed time between first and last valid observations"));
 }
 return out;
}
} // namespace bluesky::insurance
