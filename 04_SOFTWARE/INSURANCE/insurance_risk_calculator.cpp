#include "insurance_risk_calculator.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
#include <utility>

namespace bluesky::insurance {
namespace {

bool parse_number(const std::string& value, double& out) {
    try {
        std::size_t consumed = 0;
        out = std::stod(value, &consumed);
        return consumed == value.size() && std::isfinite(out);
    } catch (...) {
        return false;
    }
}

std::string metric_id(const std::string& uav, const std::string& suffix) {
    return uav + "-RISK-" + suffix;
}

RiskDerivedMetric make_metric(const std::string& id, const std::string& uav,
                              RiskDimension dimension, RiskMetricKind kind,
                              double value, const std::string& unit,
                              std::uint64_t samples, double denominator,
                              const std::string& method, std::int64_t at,
                              const std::vector<std::string>& sources,
                              bool evidence_valid) {
    RiskDerivedMetric m;
    m.metric_id = id;
    m.uav_id = uav;
    m.dimension = dimension;
    m.kind = kind;
    m.value = value;
    m.unit = unit;
    m.sample_count = samples;
    m.exposure_denominator = denominator;
    m.calculation_method = method;
    m.method_version = "1.0";
    m.calculated_at_epoch = at;
    m.source_record_ids = sources;
    m.evidence_valid = evidence_valid;
    m.authoritative = evidence_valid && !sources.empty() && !method.empty() && !m.method_version.empty();
    return m;
}

} // namespace

InsuranceRiskCalculator::InsuranceRiskCalculator(std::string uav_id) : uav_id_(std::move(uav_id)) {}

std::vector<RiskDerivedMetric> InsuranceRiskCalculator::calculate(
    const InsuranceUavRiskVector& risk, std::int64_t calculated_at_epoch) const {
    std::vector<RiskDerivedMetric> result;
    if (uav_id_.empty() || risk.uav_id() != uav_id_) return result;

    struct Group {
        RiskDimension dimension;
        std::string unit;
        double sum{0.0};
        double maximum{0.0};
        std::uint64_t count{0};
        std::vector<std::string> sources;
    };

    std::vector<Group> groups;
    for (const auto& o : risk.observations()) {
        if (o.uav_id != uav_id_ || !o.evidence_valid || o.source_record_id.empty() ||
            o.method.empty() || o.method_version.empty() || o.unit.empty()) continue;
        double value = 0.0;
        if (!parse_number(o.value, value) || value < 0.0) continue;

        auto it = std::find_if(groups.begin(), groups.end(), [&](const Group& g) {
            return g.dimension == o.dimension && g.unit == o.unit;
        });
        if (it == groups.end()) {
            groups.push_back({o.dimension, o.unit, value, value, 1, {o.source_record_id}});
        } else {
            it->sum += value;
            it->maximum = std::max(it->maximum, value);
            ++it->count;
            it->sources.push_back(o.source_record_id);
        }
    }

    for (const auto& g : groups) {
        result.push_back(make_metric(
            metric_id(uav_id_, std::to_string(static_cast<int>(g.dimension)) + "-" + g.unit + "-MEAN"),
            uav_id_, g.dimension, RiskMetricKind::Mean, g.sum / static_cast<double>(g.count),
            g.unit, g.count, 0.0,
            "Arithmetic mean of valid same-dimension, same-unit observations", calculated_at_epoch,
            g.sources, true));
        result.push_back(make_metric(
            metric_id(uav_id_, std::to_string(static_cast<int>(g.dimension)) + "-" + g.unit + "-MAX"),
            uav_id_, g.dimension, RiskMetricKind::Maximum, g.maximum, g.unit, g.count, 0.0,
            "Maximum of valid same-dimension, same-unit observations", calculated_at_epoch,
            g.sources, true));
    }

    double total_hours = 0.0;
    double bvlos_hours = 0.0;
    double adverse_hours = 0.0;
    double altitude_hours = 0.0;
    std::uint64_t total_cycles = 0;
    std::vector<std::string> exposure_sources;

    for (const auto& e : risk.exposures()) {
        if (e.uav_id != uav_id_ || !e.evidence_valid || e.source_record_id.empty() ||
            e.flight_hours < 0.0 || e.altitude_hours < 0.0 || e.adverse_weather_hours < 0.0) {
            continue;
        }
        total_hours += e.flight_hours;
        total_cycles += e.cycles;
        if (e.bvlos) bvlos_hours += e.flight_hours;
        adverse_hours += e.adverse_weather_hours;
        altitude_hours += e.altitude_hours;
        exposure_sources.push_back(e.source_record_id);
    }

    if (total_hours > 0.0 && !exposure_sources.empty()) {
        const auto common = "Exposure-normalized indicator over valid UAV exposure records";
        result.push_back(make_metric(
            metric_id(uav_id_, "BVLOS-RATIO"), uav_id_, RiskDimension::Operational,
            RiskMetricKind::BvlosExposureRatio, bvlos_hours / total_hours, "ratio",
            static_cast<std::uint64_t>(exposure_sources.size()), total_hours, common,
            calculated_at_epoch, exposure_sources, true));
        result.push_back(make_metric(
            metric_id(uav_id_, "ADVERSE-WEATHER-RATIO"), uav_id_, RiskDimension::Environmental,
            RiskMetricKind::AdverseWeatherExposureRatio, adverse_hours / total_hours, "ratio",
            static_cast<std::uint64_t>(exposure_sources.size()), total_hours, common,
            calculated_at_epoch, exposure_sources, true));
        result.push_back(make_metric(
            metric_id(uav_id_, "ALTITUDE-HOURS-RATIO"), uav_id_, RiskDimension::Operational,
            RiskMetricKind::AltitudeExposureRatio, altitude_hours / total_hours, "ratio",
            static_cast<std::uint64_t>(exposure_sources.size()), total_hours, common,
            calculated_at_epoch, exposure_sources, true));
        result.push_back(make_metric(
            metric_id(uav_id_, "CYCLES-PER-FLIGHT-HOUR"), uav_id_, RiskDimension::Technical,
            RiskMetricKind::CyclesPerFlightHour, static_cast<double>(total_cycles) / total_hours,
            "cycles/hour", static_cast<std::uint64_t>(exposure_sources.size()), total_hours,
            "Total cycles divided by total valid flight hours", calculated_at_epoch,
            exposure_sources, true));
    }

    return result;
}

} // namespace bluesky::insurance
