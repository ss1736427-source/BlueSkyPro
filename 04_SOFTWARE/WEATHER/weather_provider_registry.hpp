#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace bluesky::weather {

enum class WeatherProductType {
    AerodromeObservation,
    AerodromeForecast,
    AviationHazard,
    UpperWindTemperature,
    NumericalWindField,
    NumericalTemperatureField
};

struct GeographicCoverage {
    double west{-180.0};
    double south{-90.0};
    double east{180.0};
    double north{90.0};
};

struct WeatherProviderCapability {
    WeatherProductType product{WeatherProductType::NumericalWindField};
    GeographicCoverage coverage;
    double minimumAltitudeAglM{0.0};
    double maximumAltitudeAglM{0.0};
    std::int64_t maximumForecastHorizonSeconds{0};
    double minimumGridSpacingDegrees{0.0};
};

struct WeatherProviderDescriptor {
    std::string adapterId;
    std::string displayName;
    std::uint32_t interfaceVersion{1};
    std::vector<WeatherProviderCapability> capabilities;
};

struct WeatherProviderInstance {
    std::string instanceId;
    std::string adapterId;
    std::string displayName;
    bool enabled{false};
    int priority{100};
};

struct WeatherProviderRequest {
    WeatherProductType product{WeatherProductType::NumericalWindField};
    GeographicCoverage area;
    double minimumAltitudeAglM{0.0};
    double maximumAltitudeAglM{0.0};
    std::int64_t referenceTimeUtcSeconds{0};
    std::int64_t validFromUtcSeconds{0};
    std::int64_t validToUtcSeconds{0};
    double maximumGridSpacingDegrees{0.0};
};

class IWeatherProviderAdapter {
public:
    virtual ~IWeatherProviderAdapter() = default;
    virtual WeatherProviderDescriptor descriptor() const = 0;
};

struct WeatherProviderSelection {
    WeatherProviderInstance instance;
    WeatherProviderDescriptor adapter;
    std::size_t capabilityIndex{0};
};

class WeatherProviderRegistry final {
public:
    bool registerAdapter(std::shared_ptr<const IWeatherProviderAdapter> adapter,
                         std::string *error = nullptr);
    bool upsertInstance(WeatherProviderInstance instance,
                        std::string *error = nullptr);
    bool setEnabled(const std::string &instanceId, bool enabled);
    std::vector<WeatherProviderSelection> selectEligible(
        const WeatherProviderRequest &request) const;
    std::vector<WeatherProviderInstance> instances() const;

private:
    std::vector<std::shared_ptr<const IWeatherProviderAdapter>> m_adapters;
    std::vector<WeatherProviderInstance> m_instances;
};

} // namespace bluesky::weather
