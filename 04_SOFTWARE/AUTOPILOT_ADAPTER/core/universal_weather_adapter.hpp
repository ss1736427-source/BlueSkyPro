#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace bluesky::weather {

enum class WeatherState { Unknown, Nominal, Warning, Severe, Fault };
enum class WeatherQuality { Unknown, Invalid, Valid, Stale, Degraded };
enum class WeatherError { None, InvalidData, StaleData, ConfigurationMismatch };

struct WeatherSample {
    std::string sourceId;
    std::int64_t sourceTimestampMs{0};
    double windSpeedMps{0.0};
    double windDirectionDeg{0.0};
    double temperatureC{0.0};
    double pressureHpa{0.0};
    double humidityPercent{0.0};
    WeatherState state{WeatherState::Unknown};
    WeatherQuality quality{WeatherQuality::Unknown};
    bool valid{false};
    bool stale{false};
};

struct WeatherResult {
    bool accepted{false};
    WeatherError error{WeatherError::None};
    std::string reason;
};

class UniversalWeatherAdapter {
public:
    virtual ~UniversalWeatherAdapter() = default;
    virtual std::string getSourceId() const = 0;
    virtual std::string getWeatherIdentity() const = 0;
    virtual std::string getConfigurationVersion() const = 0;
    virtual WeatherState getState() const = 0;
    virtual WeatherQuality getQuality() const = 0;
    virtual std::optional<WeatherSample> readSample() const = 0;
    virtual WeatherResult acceptSample(const WeatherSample& sample) = 0;
};

} // namespace bluesky::weather
