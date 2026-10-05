#include "insurance_component_lifecycle.hpp"

#include <algorithm>
#include <cmath>
#include <map>
#include <utility>

namespace bluesky::insurance {
namespace {
struct Group {
    std::string component_id;
    std::string component_type;
    std::string configuration_id;
    std::vector<ComponentLifecycleObservation> values;
};

bool valid(const ComponentLifecycleObservation& o) {
    return !o.observation_id.empty() && !o.uav_id.empty() && !o.component_id.empty() &&
           !o.component_type.empty() && !o.configuration_id.empty() &&
           !o.source_record_id.empty() && !o.evidence_ref.empty() &&
           !o.method.empty() && !o.method_version.empty() && o.evidence_valid &&
           std::isfinite(o.operating_hours) && o.operating_hours >= 0.0;
}

std::string metric_id(const std::string& uav, const std::string& component,
                      const std::string& configuration) {
    return uav + "-CMP-" + component + "-" + configuration + "-LIFECYCLE";
}
}

InsuranceComponentLifecycleCalculator::InsuranceComponentLifecycleCalculator(std::string uav_id)
    : uav_id_(std::move(uav_id)) {}

std::vector<ComponentLifecycleMetric> InsuranceComponentLifecycleCalculator::calculate(
    const std::vector<ComponentLifecycleObservation>& observations,
    std::int64_t calculated_at_epoch) const {
    std::vector<ComponentLifecycleMetric> out;
    if (uav_id_.empty()) return out;

    std::map<std::string, Group> groups;
    for (const auto& o : observations) {
        if (o.uav_id != uav_id_ || !valid(o)) continue;
        const auto key = o.component_id + "|" + o.configuration_id;
        auto& g = groups[key];
        g.component_id = o.component_id;
        g.component_type = o.component_type;
        g.configuration_id = o.configuration_id;
        g.values.push_back(o);
    }

    for (auto& [key, g] : groups) {
        std::sort(g.values.begin(), g.values.end(),
                  [](const auto& a, const auto& b) {
                      return a.timestamp_epoch < b.timestamp_epoch;
                  });
        if (g.values.size() < 2) continue;

        const auto& first = g.values.front();
        const auto& last = g.values.back();
        if (last.operating_hours < first.operating_hours ||
            last.cycles < first.cycles) {
            continue;
        }

        ComponentLifecycleMetric m;
        m.metric_id = metric_id(uav_id_, g.component_id, g.configuration_id);
        m.uav_id = uav_id_;
        m.component_id = g.component_id;
        m.component_type = g.component_type;
        m.configuration_id = g.configuration_id;
        m.operating_hours_delta = last.operating_hours - first.operating_hours;
        m.cycle_delta = last.cycles - first.cycles;
        m.observation_count = g.values.size();
        m.latest_status = last.status;
        m.first_observation_epoch = first.timestamp_epoch;
        m.last_observation_epoch = last.timestamp_epoch;
        m.calculation_method = "Last valid resource observation minus first valid resource observation";
        m.method_version = "1.0";
        m.calculated_at_epoch = calculated_at_epoch;
        m.evidence_valid = true;
        m.authoritative = true;

        for (const auto& v : g.values) {
            m.source_record_ids.push_back(v.source_record_id);
            m.evidence_refs.push_back(v.evidence_ref);
        }
        out.push_back(std::move(m));
    }
    return out;
}

}  // namespace bluesky::insurance
