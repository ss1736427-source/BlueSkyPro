#include "insurance_risk_temporal.hpp"
#include <algorithm>
#include <cmath>
#include <map>
#include <utility>

namespace bluesky::insurance {
namespace {

struct ObservationGroup {
    RiskDimension dimension;
    std::string unit;
    double baseline_sum{0.0};
    double current_sum{0.0};
    std::uint64_t baseline_count{0};
    std::uint64_t current_count{0};
    std::vector<std::string> sources;
};

struct ExposureGroup {
    std::string configuration_id;
    double baseline_hours{0.0};
    double current_hours{0.0};
    double baseline_bvlos{0.0};
    double current_bvlos{0.0};
    double baseline_adverse{0.0};
    double current_adverse{0.0};
    double baseline_altitude{0.0};
    double current_altitude{0.0};
    std::uint64_t baseline_cycles{0};
    std::uint64_t current_cycles{0};
    std::vector<std::string> sources;
};

bool valid_window(const RiskTimeWindow& w) {
    return w.start_epoch < w.end_epoch;
}

bool in_window(std::int64_t timestamp, const RiskTimeWindow& w) {
    return timestamp >= w.start_epoch && timestamp < w.end_epoch;
}

std::string metric_id(const std::string& uav, const std::string& suffix) {
    return uav + "-RISK-TEMP-" + suffix;
}

bool finite_nonnegative(double v) {
    return std::isfinite(v) && v >= 0.0;
}

TemporalRiskMetric make_metric(
    const std::string& id, const std::string& uav, const std::string& config,
    RiskDimension dimension, TemporalMetricKind kind, double value,
    const std::string& unit, RiskTimeWindow baseline, RiskTimeWindow current,
    std::uint64_t baseline_count, std::uint64_t current_count,
    double baseline_value, double current_value, double denominator,
    const std::string& method, std::int64_t at,
    const std::vector<std::string>& sources) {
    TemporalRiskMetric m;
    m.metric_id = id;
    m.uav_id = uav;
    m.configuration_id = config;
    m.dimension = dimension;
    m.kind = kind;
    m.value = value;
    m.unit = unit;
    m.baseline = baseline;
    m.current = current;
    m.baseline_sample_count = baseline_count;
    m.current_sample_count = current_count;
    m.baseline_value = baseline_value;
    m.current_value = current_value;
    m.exposure_denominator = denominator;
    m.calculation_method = method;
    m.method_version = "1.0";
    m.calculated_at_epoch = at;
    m.source_record_ids = sources;
    m.evidence_valid = !sources.empty();
    m.authoritative = m.evidence_valid && !method.empty() && !m.method_version.empty();
    return m;
}

} // namespace

InsuranceRiskTemporalCalculator::InsuranceRiskTemporalCalculator(std::string uav_id)
    : uav_id_(std::move(uav_id)) {}

std::vector<TemporalRiskMetric> InsuranceRiskTemporalCalculator::calculate(
    const InsuranceUavRiskVector& risk,
    RiskTimeWindow baseline,
    RiskTimeWindow current,
    std::int64_t calculated_at_epoch) const {
    std::vector<TemporalRiskMetric> result;
    if (uav_id_.empty() || risk.uav_id() != uav_id_ ||
        !valid_window(baseline) || !valid_window(current) ||
        baseline.end_epoch > current.start_epoch) {
        return result;
    }

    std::vector<ObservationGroup> observations;
    for (const auto& o : risk.observations()) {
        if (o.uav_id != uav_id_ || !o.evidence_valid || o.source_record_id.empty() ||
            o.unit.empty() || o.method.empty() || o.method_version.empty()) continue;
        double value = 0.0;
        try {
            std::size_t consumed = 0;
            value = std::stod(o.value, &consumed);
            if (consumed != o.value.size() || !std::isfinite(value)) continue;
        } catch (...) {
            continue;
        }

        auto it = std::find_if(observations.begin(), observations.end(), [&](const auto& g) {
            return g.dimension == o.dimension && g.unit == o.unit;
        });
        if (it == observations.end()) {
            observations.push_back({o.dimension, o.unit});
            it = std::prev(observations.end());
        }
        if (in_window(o.timestamp_epoch, baseline)) {
            it->baseline_sum += value;
            ++it->baseline_count;
            it->sources.push_back(o.source_record_id);
        } else if (in_window(o.timestamp_epoch, current)) {
            it->current_sum += value;
            ++it->current_count;
            it->sources.push_back(o.source_record_id);
        }
    }

    const double baseline_duration = static_cast<double>(baseline.end_epoch - baseline.start_epoch);
    const double current_duration = static_cast<double>(current.end_epoch - current.start_epoch);

    for (const auto& g : observations) {
        if (g.baseline_count == 0 || g.current_count == 0) continue;
        const double b = g.baseline_sum / static_cast<double>(g.baseline_count);
        const double c = g.current_sum / static_cast<double>(g.current_count);
        const double delta = c - b;
        const double slope = delta / ((baseline.end_epoch + current.end_epoch -
                                       baseline.start_epoch - current.start_epoch) / 2.0);
        const std::string prefix = std::to_string(static_cast<int>(g.dimension)) + "-" + g.unit;
        result.push_back(make_metric(
            metric_id(uav_id_, prefix + "-DELTA"), uav_id_, "", g.dimension,
            TemporalMetricKind::ObservationMeanDelta, delta, g.unit, baseline, current,
            g.baseline_count, g.current_count, b, c, 0.0,
            "Current-window mean minus baseline-window mean for valid observations",
            calculated_at_epoch, g.sources));
        result.push_back(make_metric(
            metric_id(uav_id_, prefix + "-RATE"), uav_id_, "", g.dimension,
            TemporalMetricKind::ObservationRateOfChange, slope, g.unit + "/second",
            baseline, current, g.baseline_count, g.current_count, b, c, 0.0,
            "Mean delta divided by the elapsed midpoint time between non-overlapping windows",
            calculated_at_epoch, g.sources));
    }

    std::map<std::string, ExposureGroup> exposures;
    for (const auto& e : risk.exposures()) {
        if (e.uav_id != uav_id_ || !e.evidence_valid || e.source_record_id.empty() ||
            !finite_nonnegative(e.flight_hours) || !finite_nonnegative(e.altitude_hours) ||
            !finite_nonnegative(e.adverse_weather_hours) || e.configuration_id.empty()) continue;

        auto& g = exposures[e.configuration_id];
        g.configuration_id = e.configuration_id;
        if (in_window(e.timestamp_epoch, baseline)) {
            g.baseline_hours += e.flight_hours;
            g.baseline_cycles += e.cycles;
            if (e.bvlos) g.baseline_bvlos += e.flight_hours;
            g.baseline_adverse += e.adverse_weather_hours;
            g.baseline_altitude += e.altitude_hours;
            g.sources.push_back(e.source_record_id);
        } else if (in_window(e.timestamp_epoch, current)) {
            g.current_hours += e.flight_hours;
            g.current_cycles += e.cycles;
            if (e.bvlos) g.current_bvlos += e.flight_hours;
            g.current_adverse += e.adverse_weather_hours;
            g.current_altitude += e.altitude_hours;
            g.sources.push_back(e.source_record_id);
        }
    }

    for (const auto& [config, g] : exposures) {
        if (g.baseline_hours <= 0.0 || g.current_hours <= 0.0) continue;
        const double baseline_exposure = g.baseline_hours;
        const double current_exposure = g.current_hours;
        result.push_back(make_metric(
            metric_id(uav_id_, config + "-HOURS-DELTA"), uav_id_, config,
            RiskDimension::Operational, TemporalMetricKind::ConfigurationExposureDelta,
            current_exposure - baseline_exposure, "flight_hours", baseline, current,
            g.baseline_cycles, g.current_cycles, baseline_exposure, current_exposure,
            baseline_exposure + current_exposure,
            "Current configuration flight-hour exposure minus baseline configuration exposure",
            calculated_at_epoch, g.sources));
        const double baseline_bvlos = g.baseline_bvlos / baseline_exposure;
        const double current_bvlos = g.current_bvlos / current_exposure;
        result.push_back(make_metric(
            metric_id(uav_id_, config + "-BVLOS-RATIO"), uav_id_, config,
            RiskDimension::Operational, TemporalMetricKind::ConfigurationBvlosRatio,
            current_bvlos, "ratio", baseline, current,
            g.baseline_cycles, g.current_cycles, baseline_bvlos, current_bvlos,
            current_exposure,
            "BVLOS flight-hour exposure divided by configuration flight-hour exposure",
            calculated_at_epoch, g.sources));
        const double baseline_adverse = g.baseline_adverse / baseline_exposure;
        const double current_adverse = g.current_adverse / current_exposure;
        result.push_back(make_metric(
            metric_id(uav_id_, config + "-ADVERSE-RATIO"), uav_id_, config,
            RiskDimension::Environmental, TemporalMetricKind::ConfigurationAdverseWeatherRatio,
            current_adverse, "ratio", baseline, current,
            g.baseline_cycles, g.current_cycles, baseline_adverse, current_adverse,
            current_exposure,
            "Adverse-weather flight-hour exposure divided by configuration flight-hour exposure",
            calculated_at_epoch, g.sources));
        const double baseline_altitude = g.baseline_altitude / baseline_exposure;
        const double current_altitude = g.current_altitude / current_exposure;
        result.push_back(make_metric(
            metric_id(uav_id_, config + "-ALTITUDE-RATIO"), uav_id_, config,
            RiskDimension::Operational, TemporalMetricKind::ConfigurationAltitudeRatio,
            current_altitude, "ratio", baseline, current,
            g.baseline_cycles, g.current_cycles, baseline_altitude, current_altitude,
            current_exposure,
            "Altitude-hour exposure divided by configuration flight-hour exposure",
            calculated_at_epoch, g.sources));
    }

    (void)baseline_duration;
    (void)current_duration;
    return result;
}

} // namespace bluesky::insurance
