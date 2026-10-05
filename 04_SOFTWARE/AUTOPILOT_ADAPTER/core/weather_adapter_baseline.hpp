#pragma once

#include "universal_weather_adapter.hpp"

namespace bluesky::weather {

class WeatherAdapterBaseline final : public UniversalWeatherAdapter {
public:
    explicit WeatherAdapterBaseline(std::string sourceId = "SRC-07", std::string weatherIdentity = "DEMO-WEATHER");

    std::string getSourceId() const override;
    std::string getWeatherIdentity() const override;
    std::string getConfigurationVersion() const override;
    WeatherState getState() const override;
    WeatherQuality getQuality() const override;
    std::optional<WeatherSample> readSample() const override;
    WeatherResult acceptSample(const WeatherSample& sample) override;

private:
    std::string sourceId_;
    std::string weatherIdentity_;
    std::optional<WeatherSample> sample_;
};

} // namespace bluesky::weather
