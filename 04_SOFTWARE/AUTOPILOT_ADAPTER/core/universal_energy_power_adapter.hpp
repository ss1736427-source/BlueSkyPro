#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace bluesky::energy {

enum class EnergyState { Unknown, Nominal, Low, Critical, Depleted, Fault };
enum class EnergyQuality { Unknown, Invalid, Valid, Stale, Degraded };
enum class EnergyError { None, InvalidData, StaleData, ConfigurationMismatch, SafetyRejected };

struct EnergySample {
    std::string sourceId;
    std::int64_t sourceTimestampMs{0};
    double voltageV{0.0};
    double currentA{0.0};
    double powerW{0.0};
    double remainingPercent{0.0};
    double consumedAh{0.0};
    double estimatedRemainingSeconds{0.0};
    EnergyState state{EnergyState::Unknown};
    EnergyQuality quality{EnergyQuality::Unknown};
    bool valid{false};
    bool stale{false};
};

struct EnergyResult {
    bool accepted{false};
    EnergyError error{EnergyError::None};
    std::string reason;
};

class UniversalEnergyPowerAdapter {
public:
    virtual ~UniversalEnergyPowerAdapter() = default;
    virtual std::string getSourceId() const = 0;
    virtual std::string getBatteryIdentity() const = 0;
    virtual std::string getConfigurationVersion() const = 0;
    virtual EnergyState getState() const = 0;
    virtual EnergyQuality getQuality() const = 0;
    virtual std::optional<EnergySample> readSample() const = 0;
    virtual EnergyResult acceptSample(const EnergySample& sample) = 0;
};

} // namespace bluesky::energy
