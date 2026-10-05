#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace bluesky::propulsion {

enum class PropulsionState { Unknown, Nominal, Warning, Degraded, Fault, Stopped };
enum class PropulsionQuality { Unknown, Invalid, Valid, Stale, Degraded };
enum class PropulsionError { None, InvalidData, StaleData, ConfigurationMismatch, SafetyRejected };

struct PropulsionSample {
    std::string sourceId;
    std::int64_t sourceTimestampMs{0};
    double rpm{0.0};
    double thrustN{0.0};
    double torqueNm{0.0};
    double fuelFlowKgPerS{0.0};
    double throttlePercent{0.0};
    double temperatureC{0.0};
    double vibrationRms{0.0};
    PropulsionState state{PropulsionState::Unknown};
    PropulsionQuality quality{PropulsionQuality::Unknown};
    bool valid{false};
    bool stale{false};
};

struct PropulsionResult {
    bool accepted{false};
    PropulsionError error{PropulsionError::None};
    std::string reason;
};

class UniversalPropulsionAdapter {
public:
    virtual ~UniversalPropulsionAdapter() = default;
    virtual std::string getSourceId() const = 0;
    virtual std::string getPropulsionIdentity() const = 0;
    virtual std::string getConfigurationVersion() const = 0;
    virtual PropulsionState getState() const = 0;
    virtual PropulsionQuality getQuality() const = 0;
    virtual std::optional<PropulsionSample> readSample() const = 0;
    virtual PropulsionResult acceptSample(const PropulsionSample& sample) = 0;
};

} // namespace bluesky::propulsion
