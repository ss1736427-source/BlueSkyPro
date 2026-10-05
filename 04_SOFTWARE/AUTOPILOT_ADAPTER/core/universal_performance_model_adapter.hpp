#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace bluesky::performance {

enum class PerformanceState { Unknown, Nominal, Warning, Degraded, Fault };
enum class PerformanceQuality { Unknown, Invalid, Valid, Stale, Degraded };
enum class PerformanceError { None, InvalidData, StaleData, ConfigurationMismatch };

struct PerformanceSample {
    std::string sourceId;
    std::int64_t sourceTimestampMs{0};
    double predictedEnduranceSeconds{0.0};
    double predictedRangeMeters{0.0};
    double predictedEnergyPerMeterWh{0.0};
    double predictedPowerW{0.0};
    double observedEnduranceSeconds{0.0};
    double observedRangeMeters{0.0};
    double modelErrorPercent{0.0};
    PerformanceState state{PerformanceState::Unknown};
    PerformanceQuality quality{PerformanceQuality::Unknown};
    bool valid{false};
    bool stale{false};
};

struct PerformanceResult {
    bool accepted{false};
    PerformanceError error{PerformanceError::None};
    std::string reason;
};

class UniversalPerformanceModelAdapter {
public:
    virtual ~UniversalPerformanceModelAdapter() = default;
    virtual std::string getSourceId() const = 0;
    virtual std::string getModelIdentity() const = 0;
    virtual std::string getConfigurationVersion() const = 0;
    virtual PerformanceState getState() const = 0;
    virtual PerformanceQuality getQuality() const = 0;
    virtual std::optional<PerformanceSample> readSample() const = 0;
    virtual PerformanceResult acceptSample(const PerformanceSample& sample) = 0;
};

} // namespace bluesky::performance
