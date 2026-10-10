#include "weather_adapter_baseline.hpp"

#include <cmath>
#include <utility>

namespace bluesky::weather {

WeatherAdapterBaseline::WeatherAdapterBaseline(std::string sourceId, std::string weatherIdentity)
    : sourceId_(std::move(sourceId)), weatherIdentity_(std::move(weatherIdentity)) {}

std::string WeatherAdapterBaseline::getSourceId() const { return sourceId_; }
std::string WeatherAdapterBaseline::getWeatherIdentity() const { return weatherIdentity_; }
std::string WeatherAdapterBaseline::getConfigurationVersion() const { return "BASELINE-1"; }
WeatherState WeatherAdapterBaseline::getState() const {
    return sample_.has_value() ? sample_->state : WeatherState::Unknown;
}
WeatherQuality WeatherAdapterBaseline::getQuality() const {
    return sample_.has_value() ? sample_->quality : WeatherQuality::Unknown;
}
std::optional<WeatherSample> WeatherAdapterBaseline::readSample() const { return sample_; }

WeatherResult WeatherAdapterBaseline::acceptSample(const WeatherSample& sample) {
    if (sourceId_.empty() || sample.sourceId != sourceId_) {
        return {false, WeatherError::ConfigurationMismatch, "source identity mismatch"};
    }
    if (sample.sourceTimestampMs <= 0) {
        return {false, WeatherError::InvalidData, "invalid source timestamp"};
    }
    if (!sample.valid) {
        return {false, WeatherError::InvalidData, "sample marked invalid"};
    }
    if (sample.stale || sample.quality == WeatherQuality::Stale) {
        return {false, WeatherError::StaleData, "sample is stale"};
    }
    if (sample.quality == WeatherQuality::Unknown || sample.state == WeatherState::Unknown) {
        return {false, WeatherError::InvalidData, "sample quality/state is unknown"};
    }
    if (!std::isfinite(sample.windSpeedMps) || sample.windSpeedMps < 0.0 ||
        !std::isfinite(sample.windDirectionDeg) || sample.windDirectionDeg < 0.0 || sample.windDirectionDeg >= 360.0 ||
        !std::isfinite(sample.temperatureC) || sample.temperatureC < -100.0 || sample.temperatureC > 80.0 ||
        !std::isfinite(sample.pressureHpa) || sample.pressureHpa <= 0.0 ||
        !std::isfinite(sample.humidityPercent) || sample.humidityPercent < 0.0 || sample.humidityPercent > 100.0) {
        return {false, WeatherError::InvalidData, "weather value out of range"};
    }

    sample_ = sample;
    return {true, WeatherError::None, "sample accepted"};
}

} // namespace bluesky::weather
